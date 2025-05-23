#include "work_context.hpp"
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>
#include "antlog.hpp"
#include "imgui.h"

constexpr uint32_t frames_in_flight = 2;

WorkContext::WorkContext(const vkte::VulkanMainContext& vmc, vkte::VulkanCommandContext& vcc)
	: vmc(vmc), vcc(vcc), storage(vmc, vcc), swapchain(vmc, vcc, storage), ants(vmc, storage), hash_grid(vmc, storage), ui(vmc), swapchain_sync(vmc.logical_device.get())
{}

void WorkContext::construct(AppState& app_state)
{
	antlog::debug("Storage info\n{}", storage.get_memory_info());
	vcc.add_graphics_buffers(frames_in_flight);
	vcc.add_compute_buffers(2);
	vcc.add_transfer_buffers(1);
	swapchain.construct(app_state.vsync);
	app_state.get_window_extent() = swapchain.get_extent();
	for (uint32_t i = 0; i < frames_in_flight; ++i)
	{
		syncs.emplace_back(vmc.logical_device.get());
		device_timers.emplace_back(vmc);
		device_timers.back().construct(TIMER_COUNT);
		timers.emplace_back(Timer());
	}
	for (vkte::Synchronization& sync : syncs) sync.construct(SEMAPHORE_COUNT, FENCE_COUNT);
	swapchain_sync.construct(swapchain.get_framebuffer_count(), 0);
	const glm::ivec2 resolution(app_state.get_render_extent().width, app_state.get_render_extent().height);
	ants.setup_storage(app_state);
	hash_grid.setup_storage(app_state);
	ants.construct(swapchain.get_render_pass(), app_state);
	hash_grid.construct(swapchain.get_render_pass(), app_state);
	ui.construct(vcc, swapchain.get_render_pass(), frames_in_flight);
	vk::CommandBuffer& cb = vcc.get_one_time_compute_buffer();
	ants.clear(cb, app_state);
	hash_grid.clear(cb, app_state);
	vcc.submit_compute(cb, true);
}

void WorkContext::destruct()
{
	vmc.logical_device.get().waitIdle();
	for (vkte::Synchronization& sync : syncs) sync.destruct();
	swapchain_sync.destruct();
	for (auto& device_timer : device_timers) device_timer.destruct();
	syncs.clear();
	swapchain.destruct();
	ants.destruct();
	hash_grid.destruct();
	ui.destruct();
}

void WorkContext::draw_frame(AppState& app_state)
{
	syncs[app_state.current_frame].wait_for_fence(F_RENDER_FINISHED);
	syncs[app_state.current_frame].reset_fence(F_RENDER_FINISHED);
	if (app_state.total_frames > frames_in_flight)
	{
		for (int i = 0; i < TIMER_COUNT; i++) app_state.device_timings[i] = device_timers[app_state.current_frame].get_result_by_idx(i);
		app_state.frame_time = timers[app_state.current_frame].restart() / float(frames_in_flight);
		app_state.total_time += app_state.frame_time;
	}
	else timers[app_state.current_frame].restart();
	vk::ResultValue<uint32_t> image_idx = vmc.logical_device.get().acquireNextImageKHR(swapchain.get(), uint64_t(-1), syncs[app_state.current_frame].get_semaphore(S_IMAGE_AVAILABLE));
	VKTE_CHECK(image_idx.result, "Failed to acquire next image!");
	render(image_idx.value, app_state);
	app_state.current_frame = (app_state.current_frame + 1) % frames_in_flight;
	app_state.total_frames++;
}

vk::Extent2D WorkContext::resize(bool vsync)
{
	vmc.logical_device.get().waitIdle();
	swapchain.recreate(vsync);
	for (vkte::Synchronization& sync : syncs) sync.destruct();
	for (vkte::Synchronization& sync : syncs) sync.construct(SEMAPHORE_COUNT, FENCE_COUNT);
	swapchain_sync.destruct();
	swapchain_sync.construct(swapchain.get_framebuffer_count(), 0);
	return swapchain.get_extent();
}

void WorkContext::render_ui(vk::CommandBuffer& cb, AppState& app_state)
{
	ui.new_frame("AntSim");
	ImGui::PushItemWidth(80.0f);

	constexpr float update_weight = 0.1f;
	app_state.frame_time_ema = app_state.frame_time_ema * (1 - update_weight) + app_state.frame_time * update_weight;
	ImGui::Text("%.4f ms; FPS: %.2f", app_state.frame_time_ema * 1000, 1.0 / app_state.frame_time_ema);
	ImGui::Text("total time: %.4f s", app_state.total_time);
	ImGui::Text("RENDERING_ALL: %.4f ms", app_state.device_timings[TIMER_RENDERING_ALL]);
	ImGui::Text("ANTS_STEP: %.4f ms", app_state.device_timings[TIMER_ANTS_STEP]);
	ImGui::Text("HASH_GRID_STEP: %.4f ms", app_state.device_timings[TIMER_HASH_GRID_STEP]);
	ui.end_frame(cb);
}

void WorkContext::render(uint32_t image_idx, AppState& app_state)
{
	vk::CommandBuffer& compute_cb = vcc.begin(vcc.compute_cbs[app_state.current_frame]);
	device_timers[app_state.current_frame].reset(compute_cb, TIMER_ANTS_STEP);
	device_timers[app_state.current_frame].reset(compute_cb, TIMER_HASH_GRID_STEP);
	device_timers[app_state.current_frame].start(compute_cb, TIMER_ANTS_STEP, vk::PipelineStageFlagBits::eComputeShader);
	ants.compute(compute_cb, app_state);
	device_timers[app_state.current_frame].stop(compute_cb, TIMER_ANTS_STEP, vk::PipelineStageFlagBits::eComputeShader);
	const vkte::Buffer& buffer = storage.get_buffer_by_name("hash_grid_buffer");
	vk::BufferMemoryBarrier barrier(vk::AccessFlagBits::eMemoryWrite, vk::AccessFlagBits::eMemoryRead, vmc.queue_families.get(QueueFamilyFlags::Compute), vmc.queue_families.get(QueueFamilyFlags::Compute), buffer.get(), 0, buffer.get_byte_size());
	compute_cb.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader, vk::DependencyFlagBits::eDeviceGroup, {}, {barrier}, {});
	device_timers[app_state.current_frame].start(compute_cb, TIMER_HASH_GRID_STEP, vk::PipelineStageFlagBits::eComputeShader);
	hash_grid.compute(compute_cb, app_state);
	device_timers[app_state.current_frame].stop(compute_cb, TIMER_HASH_GRID_STEP, vk::PipelineStageFlagBits::eComputeShader);
	compute_cb.end();
	std::vector<vk::Semaphore> compute_wait_semaphores;
	std::vector<vk::PipelineStageFlags> compute_wait_stages;
	if (app_state.total_frames > 0)
	{
		compute_wait_semaphores.push_back(syncs[1 - app_state.current_frame].get_semaphore(S_FRAME_TO_FRAME));
		compute_wait_stages.push_back(vk::PipelineStageFlagBits::eComputeShader);
	}
	std::vector<vk::Semaphore> compute_signal_semaphores;
	compute_signal_semaphores.push_back(syncs[app_state.current_frame].get_semaphore(S_ANTS_STEP_FINISHED));
	vk::SubmitInfo compute_si(compute_wait_semaphores.size(), compute_wait_semaphores.data(), compute_wait_stages.data(), 1, &compute_cb, compute_signal_semaphores.size(), compute_signal_semaphores.data());
	vmc.get_compute_queue().submit(compute_si);

	vk::CommandBuffer& cb = vcc.begin(vcc.graphics_cbs[app_state.current_frame]);
	device_timers[app_state.current_frame].reset(cb, TIMER_RENDERING_ALL);
	device_timers[app_state.current_frame].start(cb, TIMER_RENDERING_ALL, vk::PipelineStageFlagBits::eTopOfPipe);
	vk::RenderPassBeginInfo rpbi{};
	rpbi.sType = vk::StructureType::eRenderPassBeginInfo;
	rpbi.renderPass = swapchain.get_render_pass().get();
	rpbi.framebuffer = swapchain.get_framebuffer(image_idx);
	rpbi.renderArea.offset = vk::Offset2D(0, 0);
	rpbi.renderArea.extent = swapchain.get_extent();
	std::vector<vk::ClearValue> clear_values(2);
	clear_values[0].color = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
	clear_values[1].depthStencil.depth = 1.0f;
	clear_values[1].depthStencil.stencil = 0;
	rpbi.clearValueCount = clear_values.size();
	rpbi.pClearValues = clear_values.data();
	cb.beginRenderPass(rpbi, vk::SubpassContents::eInline);

	vk::Viewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = swapchain.get_extent().width;
	viewport.height = swapchain.get_extent().height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	cb.setViewport(0, viewport);
	vk::Rect2D scissor{};
	scissor.offset = vk::Offset2D(0, 0);
	scissor.extent = swapchain.get_extent();
	cb.setScissor(0, scissor);

	ants.render(cb, app_state, swapchain.get_framebuffer(image_idx), swapchain.get_render_pass().get());
	hash_grid.render(cb, app_state, swapchain.get_framebuffer(image_idx), swapchain.get_render_pass().get());
	if (app_state.show_ui) render_ui(cb, app_state);
	cb.endRenderPass();
	device_timers[app_state.current_frame].stop(cb, TIMER_RENDERING_ALL, vk::PipelineStageFlagBits::eBottomOfPipe);
	cb.end();

	std::vector<vk::Semaphore> render_wait_semaphores;
	std::vector<vk::PipelineStageFlags> render_wait_stages;
	render_wait_semaphores.push_back(syncs[app_state.current_frame].get_semaphore(S_IMAGE_AVAILABLE));
	render_wait_semaphores.push_back(syncs[app_state.current_frame].get_semaphore(S_ANTS_STEP_FINISHED));
	render_wait_stages.push_back(vk::PipelineStageFlagBits::eColorAttachmentOutput);
	render_wait_stages.push_back(vk::PipelineStageFlagBits::eVertexInput);
	std::vector<vk::Semaphore> render_signal_semaphores;
	render_signal_semaphores.push_back(swapchain_sync.get_semaphore(image_idx));
	render_signal_semaphores.push_back(syncs[app_state.current_frame].get_semaphore(S_FRAME_TO_FRAME));
	vk::SubmitInfo render_si(render_wait_semaphores.size(), render_wait_semaphores.data(), render_wait_stages.data(), 1, &vcc.graphics_cbs[app_state.current_frame], render_signal_semaphores.size(), render_signal_semaphores.data());
	vmc.get_graphics_queue().submit(render_si, syncs[app_state.current_frame].get_fence(F_RENDER_FINISHED));

	vk::PresentInfoKHR present_info(1, &swapchain_sync.get_semaphore(image_idx), 1, &swapchain.get(), &image_idx);
	VKTE_CHECK(vmc.get_present_queue().presentKHR(present_info), "Failed to present image!");
}
