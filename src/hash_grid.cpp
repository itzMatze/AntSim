#include "hash_grid.hpp"
#include "vk/descriptor_set_handler.hpp"
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace ve
{
HashGrid::HashGrid(const VulkanMainContext& vmc, Storage& storage) : vmc(vmc), storage(storage)
{
	pipeline_data[CLEAR_PIPELINE] = std::make_unique<PipelineData>(PipelineData{Pipeline(vmc), DescriptorSetHandler(vmc, 1)});
	pipeline_data[RENDER_PIPELINE] = std::make_unique<PipelineData>(PipelineData{Pipeline(vmc), DescriptorSetHandler(vmc, 1)});
	pipeline_data[STEP_PIPELINE] = std::make_unique<PipelineData>(PipelineData{Pipeline(vmc), DescriptorSetHandler(vmc, 1)});
}

void HashGrid::setup_storage(AppState& app_state)
{
	std::vector<HashGridCellData> hash_grid_data(app_state.hash_grid_capacity);
	buffers[HASH_GRID_BUFFER] = storage.add_buffer("hash_grid_buffer", hash_grid_data, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc, true, vmc.queue_family_indices.transfer, vmc.queue_family_indices.compute, vmc.queue_family_indices.graphics);
}

void HashGrid::construct(const RenderPass& render_pass, AppState& app_state)
{
	create_descriptor_set();
	create_pipelines(render_pass, app_state);
}

void HashGrid::destruct()
{
	for (int32_t i : buffers) storage.destroy_buffer(i);
	for (std::unique_ptr<PipelineData>& pipeline_datum : pipeline_data)
	{
		pipeline_datum->pipeline.destruct();
		pipeline_datum->dsh.destruct();
	}
}

void HashGrid::clear(vk::CommandBuffer& cb, AppState& app_state)
{
	cb.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline_data[CLEAR_PIPELINE]->pipeline.get());
	cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipeline_data[CLEAR_PIPELINE]->pipeline.get_layout(), 0, pipeline_data[CLEAR_PIPELINE]->dsh.get_sets()[0], {});
	cb.dispatch((app_state.hash_grid_capacity + 31) / 32, 1, 1);
}

void HashGrid::compute(vk::CommandBuffer& cb, AppState& app_state)
{
	spc.food_pos = app_state.add_food_pos;
	spc.food_amount = app_state.add_food_amount;
	spc.frame_idx = app_state.total_frames;
	spc.frame_time = app_state.frame_time;
	spc.total_time = app_state.total_time;
	cb.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline_data[STEP_PIPELINE]->pipeline.get());
	cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipeline_data[STEP_PIPELINE]->pipeline.get_layout(), 0, pipeline_data[STEP_PIPELINE]->dsh.get_sets()[0], {});
	cb.pushConstants(pipeline_data[STEP_PIPELINE]->pipeline.get_layout(), vk::ShaderStageFlagBits::eCompute, 0, sizeof(StepPushConstants), &spc);
	cb.dispatch((app_state.hash_grid_capacity + 31) / 32, 1, 1);
}

void HashGrid::render(vk::CommandBuffer& cb, AppState& app_state, const vk::Framebuffer& framebuffer, const vk::RenderPass& render_pass)
{
	rpc.range_min = app_state.visible_range_min;
	rpc.range_max = app_state.visible_range_max;
	cb.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_data[RENDER_PIPELINE]->pipeline.get());
	cb.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_data[RENDER_PIPELINE]->pipeline.get_layout(), 0, pipeline_data[RENDER_PIPELINE]->dsh.get_sets()[0], {});
	cb.pushConstants(pipeline_data[RENDER_PIPELINE]->pipeline.get_layout(), vk::ShaderStageFlagBits::eFragment, 0, sizeof(RenderPushConstants), &rpc);
	cb.draw(3, 1, 0, 0);
}

void HashGrid::create_pipelines(const RenderPass& render_pass, const AppState& app_state)
{
	{
		std::array<vk::SpecializationMapEntry, 1> spec_entries;
		spec_entries[0] = vk::SpecializationMapEntry(0, 0, sizeof(uint32_t));
		std::array<uint32_t, 1> spec_entries_data{app_state.hash_grid_capacity};
		vk::SpecializationInfo spec_info(spec_entries.size(), spec_entries.data(), sizeof(uint32_t) * spec_entries_data.size(), spec_entries_data.data());
		ShaderInfo clear_hash_grid_shader_info = ShaderInfo{"hash_grid_clear.comp", vk::ShaderStageFlagBits::eCompute, spec_info};
		Pipeline::ComputeSettings settings;
		settings.set_layout = &pipeline_data[CLEAR_PIPELINE]->dsh.get_layout();
		settings.shader_info = &clear_hash_grid_shader_info;
		settings.push_constant_byte_size = sizeof(StepPushConstants);
		pipeline_data[CLEAR_PIPELINE]->pipeline.construct(settings);
	}
	{
		std::array<vk::SpecializationMapEntry, 1> spec_entries;
		spec_entries[0] = vk::SpecializationMapEntry(0, 0, sizeof(uint32_t));
		std::array<uint32_t, 1> spec_entries_data{app_state.hash_grid_capacity};
		vk::SpecializationInfo spec_info(spec_entries.size(), spec_entries.data(), sizeof(uint32_t) * spec_entries_data.size(), spec_entries_data.data());
		std::vector<ShaderInfo> render_shader_infos(2);
		render_shader_infos[0] = ShaderInfo{"hash_grid.vert", vk::ShaderStageFlagBits::eVertex};
		render_shader_infos[1] = ShaderInfo{"hash_grid.frag", vk::ShaderStageFlagBits::eFragment, spec_info};
		Pipeline::GraphicsSettings settings;
		settings.render_pass = &render_pass;
		settings.set_layout = &pipeline_data[RENDER_PIPELINE]->dsh.get_layout();
		settings.polygon_mode = vk::PolygonMode::eFill;
		settings.primitive_topology = vk::PrimitiveTopology::eTriangleList;
		settings.shader_infos = &render_shader_infos;
		std::vector<vk::PushConstantRange> pcrs = {vk::PushConstantRange(vk::ShaderStageFlagBits::eFragment, 0, sizeof(RenderPushConstants))};
		settings.pcrs = &pcrs;
		pipeline_data[RENDER_PIPELINE]->pipeline.construct(settings);
	}
	{
		std::array<vk::SpecializationMapEntry, 1> spec_entries;
		spec_entries[0] = vk::SpecializationMapEntry(0, 0, sizeof(uint32_t));
		std::array<uint32_t, 1> spec_entries_data{app_state.hash_grid_capacity};
		vk::SpecializationInfo spec_info(spec_entries.size(), spec_entries.data(), sizeof(uint32_t) * spec_entries_data.size(), spec_entries_data.data());
		ShaderInfo hash_grid_step_shader_info = ShaderInfo{"hash_grid_step.comp", vk::ShaderStageFlagBits::eCompute, spec_info};
		Pipeline::ComputeSettings settings;
		settings.set_layout = &pipeline_data[STEP_PIPELINE]->dsh.get_layout();
		settings.shader_info = &hash_grid_step_shader_info;
		settings.push_constant_byte_size = sizeof(StepPushConstants);
		pipeline_data[STEP_PIPELINE]->pipeline.construct(settings);
	}
}

void HashGrid::create_descriptor_set()
{
	pipeline_data[STEP_PIPELINE]->dsh.add_binding(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute);
	pipeline_data[CLEAR_PIPELINE]->dsh.add_binding(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute);
	pipeline_data[RENDER_PIPELINE]->dsh.add_binding(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eFragment);
	pipeline_data[RENDER_PIPELINE]->dsh.add_binding(1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eFragment);
	pipeline_data[STEP_PIPELINE]->dsh.add_descriptor(0, 0, storage.get_buffer(buffers[HASH_GRID_BUFFER]));
	pipeline_data[CLEAR_PIPELINE]->dsh.add_descriptor(0, 0, storage.get_buffer(buffers[HASH_GRID_BUFFER]));
	pipeline_data[RENDER_PIPELINE]->dsh.add_descriptor(0, 0, storage.get_buffer(buffers[HASH_GRID_BUFFER]));
	pipeline_data[RENDER_PIPELINE]->dsh.add_descriptor(0, 1, storage.get_buffer_by_name("nest_buffer"));
	for (std::unique_ptr<PipelineData>& pipeline_datum : pipeline_data) pipeline_datum->dsh.construct();
}
} // namespace ve
