#include "hash_grid.hpp"
#include "vkte/descriptor_set_handler.hpp"
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>

HashGrid::HashGrid(const vkte::VulkanMainContext& vmc, vkte::Storage& storage) : vmc(vmc), storage(storage)
{}

void HashGrid::setup_storage(AppState& app_state)
{
	std::vector<HashGridCellData> hash_grid_data(app_state.hash_grid_capacity);
	buffers[HASH_GRID_BUFFER] = storage.add_buffer("hash_grid_buffer", hash_grid_data, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc, true, QueueFamilyFlags::Transfer | QueueFamilyFlags::Compute | QueueFamilyFlags::Graphics);
}

void HashGrid::construct(const vkte::RenderPass& render_pass, AppState& app_state, uint32_t frames_in_flight)
{
	pipeline_data[CLEAR_PIPELINE] = std::make_unique<PipelineData>(PipelineData{vkte::Pipeline(vmc, vkte::Pipeline::Type::Compute), vkte::DescriptorSetHandler(vmc, frames_in_flight)});
	pipeline_data[RENDER_PIPELINE] = std::make_unique<PipelineData>(PipelineData{vkte::Pipeline(vmc, vkte::Pipeline::Type::Graphics), vkte::DescriptorSetHandler(vmc, frames_in_flight)});
	pipeline_data[STEP_PIPELINE] = std::make_unique<PipelineData>(PipelineData{vkte::Pipeline(vmc, vkte::Pipeline::Type::Compute), vkte::DescriptorSetHandler(vmc, frames_in_flight)});
	create_descriptor_set(frames_in_flight);
	create_pipelines(render_pass, app_state);
}

void HashGrid::destruct()
{
	for (int32_t i : buffers) storage.destroy_buffer(i);
	for (std::unique_ptr<PipelineData>& pipeline_datum : pipeline_data)
	{
		pipeline_datum->pipeline.destruct();
		pipeline_datum->dsh.destruct();
		pipeline_datum = nullptr;
	}
}

void HashGrid::clear(vk::CommandBuffer& cb, AppState& app_state)
{
	cb.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline_data[CLEAR_PIPELINE]->pipeline.get());
	cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipeline_data[CLEAR_PIPELINE]->pipeline.get_layout(), 0, pipeline_data[CLEAR_PIPELINE]->dsh.get_sets()[app_state.current_frame], {});
	cb.dispatch((app_state.hash_grid_capacity + 31) / 32, 1, 1);
}

void HashGrid::compute(vk::CommandBuffer& cb, AppState& app_state)
{
	spc.food_pos = app_state.add_food_pos;
	spc.food_amount = app_state.add_food_amount;
	cb.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline_data[STEP_PIPELINE]->pipeline.get());
	cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipeline_data[STEP_PIPELINE]->pipeline.get_layout(), 0, pipeline_data[STEP_PIPELINE]->dsh.get_sets()[app_state.current_frame], {});
	cb.pushConstants(pipeline_data[STEP_PIPELINE]->pipeline.get_layout(), vk::ShaderStageFlagBits::eCompute, 0, sizeof(StepPushConstants), &spc);
	cb.dispatch((app_state.hash_grid_capacity + 31) / 32, 1, 1);
}

void HashGrid::render(vk::CommandBuffer& cb, AppState& app_state, const vk::Framebuffer& framebuffer, const vk::RenderPass& render_pass)
{
	cb.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_data[RENDER_PIPELINE]->pipeline.get());
	cb.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_data[RENDER_PIPELINE]->pipeline.get_layout(), 0, pipeline_data[RENDER_PIPELINE]->dsh.get_sets()[app_state.current_frame], {});
	cb.draw(3, 1, 0, 0);
}

void HashGrid::create_pipelines(const vkte::RenderPass& render_pass, const AppState& app_state)
{
	{
		vkte::Pipeline::ComputeSettings& settings = pipeline_data[CLEAR_PIPELINE]->pipeline.get_compute_settings();
		settings.shader = vkte::Shader("hash_grid_clear.comp", vk::ShaderStageFlagBits::eCompute);
		settings.shader.add_specialization_constant(0, app_state.hash_grid_capacity);
		settings.set_layout = &pipeline_data[CLEAR_PIPELINE]->dsh.get_layout();
		settings.push_constant_byte_size = 0;
	}
	{
		vkte::Pipeline::GraphicsSettings& settings = pipeline_data[RENDER_PIPELINE]->pipeline.get_graphics_settings();
		settings.shaders.push_back(vkte::Shader("hash_grid.vert", vk::ShaderStageFlagBits::eVertex));
		settings.shaders.push_back(vkte::Shader("hash_grid.frag", vk::ShaderStageFlagBits::eFragment));
		settings.shaders.back().add_specialization_constant(0, app_state.hash_grid_capacity);
		settings.render_pass = &render_pass;
		settings.set_layout = &pipeline_data[RENDER_PIPELINE]->dsh.get_layout();
		settings.polygon_mode = vk::PolygonMode::eFill;
		settings.primitive_topology = vk::PrimitiveTopology::eTriangleList;
	}
	{
		vkte::Pipeline::ComputeSettings& settings = pipeline_data[STEP_PIPELINE]->pipeline.get_compute_settings();
		settings.shader = vkte::Shader("hash_grid_step.comp", vk::ShaderStageFlagBits::eCompute);
		settings.shader.add_specialization_constant(0, app_state.hash_grid_capacity);
		settings.set_layout = &pipeline_data[STEP_PIPELINE]->dsh.get_layout();
		settings.push_constant_byte_size = sizeof(StepPushConstants);
	}
	for (std::unique_ptr<PipelineData>& pipeline_datum : pipeline_data)
	{
		VKTE_ASSERT(pipeline_datum->pipeline.compile_shaders(), "vkte: Failed to compile shaders!");
		pipeline_datum->pipeline.construct();
	}
}

void HashGrid::create_descriptor_set(uint32_t frames_in_flight)
{
	pipeline_data[STEP_PIPELINE]->dsh.add_binding(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute);
	pipeline_data[STEP_PIPELINE]->dsh.add_binding(99, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eCompute);
	pipeline_data[CLEAR_PIPELINE]->dsh.add_binding(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute);
	pipeline_data[RENDER_PIPELINE]->dsh.add_binding(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eFragment);
	pipeline_data[RENDER_PIPELINE]->dsh.add_binding(1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eFragment);
	pipeline_data[RENDER_PIPELINE]->dsh.add_binding(99, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment);
	for (uint32_t i = 0; i < frames_in_flight; ++i)
	{
		pipeline_data[STEP_PIPELINE]->dsh.add_descriptor(i, 0, storage.get_buffer(buffers[HASH_GRID_BUFFER]));
		pipeline_data[STEP_PIPELINE]->dsh.add_descriptor(i, 99, storage.get_buffer_by_name("uniform_buffer_" + std::to_string(i)));
		pipeline_data[CLEAR_PIPELINE]->dsh.add_descriptor(i, 0, storage.get_buffer(buffers[HASH_GRID_BUFFER]));
		pipeline_data[RENDER_PIPELINE]->dsh.add_descriptor(i, 0, storage.get_buffer(buffers[HASH_GRID_BUFFER]));
		pipeline_data[RENDER_PIPELINE]->dsh.add_descriptor(i, 1, storage.get_buffer_by_name("nest_buffer"));
		pipeline_data[RENDER_PIPELINE]->dsh.add_descriptor(i, 99, storage.get_buffer_by_name("uniform_buffer_" + std::to_string(i)));
	}
	for (std::unique_ptr<PipelineData>& pipeline_datum : pipeline_data) pipeline_datum->dsh.construct();
}
