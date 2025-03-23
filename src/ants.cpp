#include "ants.hpp"
#include "vk/common.hpp"
#include "vk/descriptor_set_handler.hpp"
#include <vulkan/vulkan_handles.hpp>

namespace ve
{
Ants::Ants(const VulkanMainContext& vmc, Storage& storage) : vmc(vmc), storage(storage)
{
	pipeline_data[CLEAR_PIPELINE] = std::make_unique<PipelineData>(PipelineData{Pipeline(vmc), DescriptorSetHandler(vmc, 1)});
	pipeline_data[RENDER_PIPELINE] = std::make_unique<PipelineData>(PipelineData{Pipeline(vmc), DescriptorSetHandler(vmc, frames_in_flight)});
	pipeline_data[STEP_PIPELINE] = std::make_unique<PipelineData>(PipelineData{Pipeline(vmc), DescriptorSetHandler(vmc, frames_in_flight)});
}

void Ants::setup_storage(AppState& app_state)
{
	std::vector<AntData> ants_data(ant_count);
	buffers[ANTS_BUFFER_0] = storage.add_buffer("ants_buffer_0", ants_data, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eStorageBuffer, false, vmc.queue_family_indices.transfer, vmc.queue_family_indices.compute, vmc.queue_family_indices.graphics);
	buffers[ANTS_BUFFER_1] = storage.add_buffer("ants_buffer_1", ants_data, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eStorageBuffer, false, vmc.queue_family_indices.transfer, vmc.queue_family_indices.compute, vmc.queue_family_indices.graphics);
}

void Ants::construct(const RenderPass& render_pass, AppState& app_state)
{
	// set sane default value for upper limit based on resolution and bucket count
	app_state.histogram_limit_values.y = (app_state.get_render_extent().width * app_state.get_render_extent().height * 4.0f) / app_state.histogram_bucket_count;
	create_descriptor_set();
	create_pipelines(render_pass, app_state);
}

void Ants::destruct()
{
	for (int32_t i : buffers) storage.destroy_buffer(i);
	for (std::unique_ptr<PipelineData>& pipeline_datum : pipeline_data)
	{
		pipeline_datum->pipeline.destruct();
		pipeline_datum->dsh.destruct();
	}
}

void Ants::clear(vk::CommandBuffer& cb, AppState& app_state)
{
	cb.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline_data[CLEAR_PIPELINE]->pipeline.get());
	cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipeline_data[CLEAR_PIPELINE]->pipeline.get_layout(), 0, pipeline_data[CLEAR_PIPELINE]->dsh.get_sets()[0], {});
	cb.dispatch((app_state.histogram_bucket_count + 31) / 32, 1, 1);
}

void Ants::compute(vk::CommandBuffer& cb, AppState& app_state)
{
	cb.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline_data[STEP_PIPELINE]->pipeline.get());
	cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipeline_data[STEP_PIPELINE]->pipeline.get_layout(), 0, pipeline_data[STEP_PIPELINE]->dsh.get_sets()[app_state.current_frame], {});
	cb.pushConstants(pipeline_data[STEP_PIPELINE]->pipeline.get_layout(), vk::ShaderStageFlagBits::eCompute, 0, sizeof(PushConstants), &pc);
	cb.dispatch((app_state.get_render_extent().width + 31) / 32, (app_state.get_render_extent().height + 31) / 32, 1);
}

void Ants::render(vk::CommandBuffer& cb, AppState& app_state, const vk::Framebuffer& framebuffer, const vk::RenderPass& render_pass)
{
	cb.bindVertexBuffers(0, storage.get_buffer(buffers[ANTS_BUFFER_0 + app_state.current_frame]).get(), {0});
	cb.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_data[RENDER_PIPELINE]->pipeline.get());
	cb.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_data[RENDER_PIPELINE]->pipeline.get_layout(), 0, pipeline_data[RENDER_PIPELINE]->dsh.get_sets()[app_state.current_frame], {});
	cb.draw(ant_count, 1, 0, 0);
}

void Ants::create_pipelines(const RenderPass& render_pass, const AppState& app_state)
{
	{
		std::array<vk::SpecializationMapEntry, 1> spec_entries;
		spec_entries[0] = vk::SpecializationMapEntry(0, 0, sizeof(uint32_t));
		std::array<uint32_t, 1> spec_entries_data{ant_count};
		vk::SpecializationInfo spec_info(spec_entries.size(), spec_entries.data(), sizeof(uint32_t) * spec_entries_data.size(), spec_entries_data.data());
		ShaderInfo clear_ants_shader_info = ShaderInfo{"ants_clear.comp", vk::ShaderStageFlagBits::eCompute, spec_info};
		Pipeline::ComputeSettings settings;
		settings.set_layout = &pipeline_data[CLEAR_PIPELINE]->dsh.get_layouts()[0];
		settings.shader_info = &clear_ants_shader_info;
		settings.push_constant_byte_size = sizeof(PushConstants);
		pipeline_data[CLEAR_PIPELINE]->pipeline.construct(settings);
	}
	{
		std::array<vk::SpecializationMapEntry, 1> spec_entries;
		spec_entries[0] = vk::SpecializationMapEntry(0, 0, sizeof(uint32_t));
		std::array<uint32_t, 1> spec_entries_data{5};
		vk::SpecializationInfo spec_info(spec_entries.size(), spec_entries.data(), sizeof(uint32_t) * spec_entries_data.size(), spec_entries_data.data());

		std::vector<ShaderInfo> render_shader_infos(2);
		render_shader_infos[0] = ShaderInfo{"ants.vert", vk::ShaderStageFlagBits::eVertex, spec_info};
		render_shader_infos[1] = ShaderInfo{"ants.frag", vk::ShaderStageFlagBits::eFragment};
		Pipeline::GraphicsSettings settings;
		settings.render_pass = &render_pass;
		settings.set_layout = &pipeline_data[RENDER_PIPELINE]->dsh.get_layouts()[0];
		settings.shader_infos = &render_shader_infos;
		settings.polygon_mode = vk::PolygonMode::ePoint;

		vk::VertexInputBindingDescription binding_description{};
		binding_description.binding = 0;
		binding_description.stride = sizeof(AntData);
		binding_description.inputRate = vk::VertexInputRate::eVertex;
		settings.binding_descriptions = &binding_description;

		std::vector<vk::VertexInputAttributeDescription> attribute_descriptions(1);
		attribute_descriptions[0].binding = 0;
		attribute_descriptions[0].location = 0;
		attribute_descriptions[0].format = vk::Format::eR32G32Sfloat;
		attribute_descriptions[0].offset = offsetof(AntData, pos);
		settings.attribute_description = &attribute_descriptions;

		settings.primitive_topology = vk::PrimitiveTopology::ePointList;
		settings.pcrs = nullptr;
		pipeline_data[RENDER_PIPELINE]->pipeline.construct(settings);
	}
	{
		std::array<vk::SpecializationMapEntry, 1> spec_entries;
		spec_entries[0] = vk::SpecializationMapEntry(0, 0, sizeof(uint32_t));
		std::array<uint32_t, 1> spec_entries_data{ant_count};
		vk::SpecializationInfo spec_info(spec_entries.size(), spec_entries.data(), sizeof(uint32_t) * spec_entries_data.size(), spec_entries_data.data());
		ShaderInfo ants_step_shader_info = ShaderInfo{"ants_step.comp", vk::ShaderStageFlagBits::eCompute, spec_info};
		Pipeline::ComputeSettings settings;
		settings.set_layout = &pipeline_data[STEP_PIPELINE]->dsh.get_layouts()[0];
		settings.shader_info = &ants_step_shader_info;
		settings.push_constant_byte_size = sizeof(PushConstants);
		pipeline_data[STEP_PIPELINE]->pipeline.construct(settings);
	}
}

void Ants::create_descriptor_set()
{
	pipeline_data[STEP_PIPELINE]->dsh.add_binding(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute);
	pipeline_data[STEP_PIPELINE]->dsh.add_binding(1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute);
	pipeline_data[CLEAR_PIPELINE]->dsh.add_binding(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute);
	pipeline_data[CLEAR_PIPELINE]->dsh.add_binding(1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute);
	for (uint32_t i = 0; i < frames_in_flight; ++i)
	{
		pipeline_data[STEP_PIPELINE]->dsh.add_descriptor(i, 0, storage.get_buffer(buffers[ANTS_BUFFER_0 + i]));
		pipeline_data[STEP_PIPELINE]->dsh.add_descriptor(i, 1, storage.get_buffer(buffers[ANTS_BUFFER_1 - i]));
	}
	pipeline_data[CLEAR_PIPELINE]->dsh.add_descriptor(0, 0, storage.get_buffer(buffers[ANTS_BUFFER_0]));
	pipeline_data[CLEAR_PIPELINE]->dsh.add_descriptor(0, 1, storage.get_buffer(buffers[ANTS_BUFFER_1]));
	for (std::unique_ptr<PipelineData>& pipeline_datum : pipeline_data) pipeline_datum->dsh.construct();
}
} // namespace ve
