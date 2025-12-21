#include "ants.hpp"
#include "vkte/descriptor_set_handler.hpp"
#include <vulkan/vulkan_handles.hpp>

Ants::Ants(const vkte::VulkanMainContext& vmc, vkte::Storage& storage) : vmc(vmc), storage(storage)
{}

void Ants::setup_storage(AppState& app_state)
{
	std::vector<AntData> ants_data(app_state.ant_count);
	buffers[ANTS_BUFFER] = storage.add_buffer("ants_buffer", ants_data, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eStorageBuffer, true, QueueFamilyFlags::Transfer | QueueFamilyFlags::Compute | QueueFamilyFlags::Graphics);
	NestData nest{.pos = glm::vec2(0.0f, 0.0f), .level = 1, .food_amount = 0};
	buffers[NEST_BUFFER] = storage.add_buffer("nest_buffer", &nest, 1, vk::BufferUsageFlagBits::eStorageBuffer, true, QueueFamilyFlags::Transfer | QueueFamilyFlags::Compute | QueueFamilyFlags::Graphics);
}

void Ants::construct(const vkte::RenderPass& render_pass, AppState& app_state, uint32_t frames_in_flight)
{
	pipeline_data[CLEAR_PIPELINE] = std::make_unique<PipelineData>(PipelineData{vkte::Pipeline(vmc, vkte::Pipeline::Type::Compute), vkte::DescriptorSetHandler(vmc, frames_in_flight)});
	pipeline_data[RENDER_PIPELINE] = std::make_unique<PipelineData>(PipelineData{vkte::Pipeline(vmc, vkte::Pipeline::Type::Graphics), vkte::DescriptorSetHandler(vmc, frames_in_flight)});
	pipeline_data[STEP_PIPELINE] = std::make_unique<PipelineData>(PipelineData{vkte::Pipeline(vmc, vkte::Pipeline::Type::Compute), vkte::DescriptorSetHandler(vmc, frames_in_flight)});
	pipeline_data[UPDATE_HASH_GRID_PIPELINE] = std::make_unique<PipelineData>(PipelineData{vkte::Pipeline(vmc, vkte::Pipeline::Type::Compute), vkte::DescriptorSetHandler(vmc, frames_in_flight)});
	create_descriptor_set(frames_in_flight);
	create_pipelines(render_pass, app_state);
}

void Ants::destruct()
{
	for (int32_t i : buffers) storage.destroy_buffer(i);
	for (std::unique_ptr<PipelineData>& pipeline_datum : pipeline_data)
	{
		pipeline_datum->pipeline.destruct();
		pipeline_datum->dsh.destruct();
		pipeline_datum = nullptr;
	}
}

void Ants::clear(vk::CommandBuffer& cb, AppState& app_state)
{
	cb.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline_data[CLEAR_PIPELINE]->pipeline.get());
	cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipeline_data[CLEAR_PIPELINE]->pipeline.get_layout(), 0, pipeline_data[CLEAR_PIPELINE]->dsh.get_sets()[app_state.current_frame], {});
	cb.dispatch((app_state.ant_count + 31) / 32, 1, 1);
}

void Ants::compute(vk::CommandBuffer& cb, AppState& app_state)
{
	cb.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline_data[STEP_PIPELINE]->pipeline.get());
	cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipeline_data[STEP_PIPELINE]->pipeline.get_layout(), 0, pipeline_data[STEP_PIPELINE]->dsh.get_sets()[app_state.current_frame], {});
	cb.dispatch((app_state.ant_count + 31) / 32, 1, 1);

	vk::BufferMemoryBarrier ant_barrier(vk::AccessFlagBits::eMemoryWrite, vk::AccessFlagBits::eMemoryRead, vmc.queue_families.get(QueueFamilyFlags::Compute), vmc.queue_families.get(QueueFamilyFlags::Compute), storage.get_buffer(buffers[ANTS_BUFFER]).get(), 0, storage.get_buffer(buffers[ANTS_BUFFER]).get_byte_size());
	vk::BufferMemoryBarrier hash_grid_barrier(vk::AccessFlagBits::eMemoryRead, vk::AccessFlagBits::eMemoryWrite, vmc.queue_families.get(QueueFamilyFlags::Compute), vmc.queue_families.get(QueueFamilyFlags::Compute), storage.get_buffer_by_name("hash_grid_buffer").get(), 0, storage.get_buffer_by_name("hash_grid_buffer").get_byte_size());
	cb.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader, vk::DependencyFlagBits::eDeviceGroup, {}, {ant_barrier, hash_grid_barrier}, {});

	cb.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline_data[UPDATE_HASH_GRID_PIPELINE]->pipeline.get());
	cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipeline_data[UPDATE_HASH_GRID_PIPELINE]->pipeline.get_layout(), 0, pipeline_data[UPDATE_HASH_GRID_PIPELINE]->dsh.get_sets()[app_state.current_frame], {});
	cb.dispatch((app_state.ant_count + 31) / 32, 1, 1);
}

void Ants::render(vk::CommandBuffer& cb, AppState& app_state, const vk::Framebuffer& framebuffer, const vk::RenderPass& render_pass)
{
	cb.bindVertexBuffers(0, storage.get_buffer(buffers[ANTS_BUFFER]).get(), {0});
	cb.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_data[RENDER_PIPELINE]->pipeline.get());
	cb.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_data[RENDER_PIPELINE]->pipeline.get_layout(), 0, pipeline_data[RENDER_PIPELINE]->dsh.get_sets()[app_state.current_frame], {});
	cb.draw(6, app_state.ant_count, 0, 0);
}

void Ants::create_pipelines(const vkte::RenderPass& render_pass, const AppState& app_state)
{
	{
		vkte::Pipeline::ComputeSettings& settings = pipeline_data[CLEAR_PIPELINE]->pipeline.get_compute_settings();
		settings.shader = vkte::Shader("ants_clear.comp", vk::ShaderStageFlagBits::eCompute);
		settings.shader.add_specialization_constant(0, app_state.ant_count);
		settings.set_layout = &pipeline_data[CLEAR_PIPELINE]->dsh.get_layout();
		settings.push_constant_byte_size = 0;
	}
	{
		vkte::Pipeline::GraphicsSettings& settings = pipeline_data[RENDER_PIPELINE]->pipeline.get_graphics_settings();
		settings.shaders.push_back(vkte::Shader("ants.vert", vk::ShaderStageFlagBits::eVertex));
		settings.shaders.push_back(vkte::Shader("ants.frag", vk::ShaderStageFlagBits::eFragment));
		settings.render_pass = &render_pass;
		settings.set_layout = &pipeline_data[RENDER_PIPELINE]->dsh.get_layout();
		settings.polygon_mode = vk::PolygonMode::eFill;
		settings.binding_descriptions.push_back(vk::VertexInputBindingDescription(0, sizeof(AntData), vk::VertexInputRate::eInstance));
		settings.attribute_description.push_back(vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32Sfloat, offsetof(AntData, pos)));
		settings.attribute_description.push_back(vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32Sfloat, offsetof(AntData, dir)));
		settings.attribute_description.push_back(vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32Uint, offsetof(AntData, state_bits)));
		settings.primitive_topology = vk::PrimitiveTopology::eTriangleList;
	}
	{
		vkte::Pipeline::ComputeSettings& settings = pipeline_data[STEP_PIPELINE]->pipeline.get_compute_settings();
		settings.shader = vkte::Shader("ants_step.comp", vk::ShaderStageFlagBits::eCompute);
		settings.shader.add_specialization_constant(0, app_state.ant_count);
		settings.shader.add_specialization_constant(1, app_state.hash_grid_capacity);
		settings.set_layout = &pipeline_data[STEP_PIPELINE]->dsh.get_layout();
		settings.push_constant_byte_size = 0;
	}
	{
		vkte::Pipeline::ComputeSettings& settings = pipeline_data[UPDATE_HASH_GRID_PIPELINE]->pipeline.get_compute_settings();
		settings.shader = vkte::Shader("ants_update_hash_grid.comp", vk::ShaderStageFlagBits::eCompute);
		settings.shader.add_specialization_constant(0, app_state.ant_count);
		settings.shader.add_specialization_constant(1, app_state.hash_grid_capacity);
		settings.set_layout = &pipeline_data[UPDATE_HASH_GRID_PIPELINE]->dsh.get_layout();
		settings.push_constant_byte_size = 0;
	}
	for (std::unique_ptr<PipelineData>& pipeline_datum : pipeline_data)
	{
		VKTE_ASSERT(pipeline_datum->pipeline.compile_shaders(), "vkte: Failed to compile shaders!");
		pipeline_datum->pipeline.construct();
	}
}

void Ants::create_descriptor_set(uint32_t frames_in_flight)
{
	pipeline_data[STEP_PIPELINE]->dsh.add_binding(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute);
	pipeline_data[STEP_PIPELINE]->dsh.add_binding(1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute);
	pipeline_data[STEP_PIPELINE]->dsh.add_binding(2, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute);
	pipeline_data[STEP_PIPELINE]->dsh.add_binding(99, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eCompute);
	pipeline_data[UPDATE_HASH_GRID_PIPELINE]->dsh.add_binding(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute);
	pipeline_data[UPDATE_HASH_GRID_PIPELINE]->dsh.add_binding(1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute);
	pipeline_data[UPDATE_HASH_GRID_PIPELINE]->dsh.add_binding(99, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eCompute);
	pipeline_data[CLEAR_PIPELINE]->dsh.add_binding(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute);
	pipeline_data[RENDER_PIPELINE]->dsh.add_binding(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eFragment);
	pipeline_data[RENDER_PIPELINE]->dsh.add_binding(99, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);
	for (uint32_t i = 0; i < frames_in_flight; ++i)
	{
		pipeline_data[STEP_PIPELINE]->dsh.add_descriptor(i, 0, storage.get_buffer(buffers[ANTS_BUFFER]));
		pipeline_data[STEP_PIPELINE]->dsh.add_descriptor(i, 1, storage.get_buffer_by_name("hash_grid_buffer"));
		pipeline_data[STEP_PIPELINE]->dsh.add_descriptor(i, 2, storage.get_buffer(buffers[NEST_BUFFER]));
		pipeline_data[STEP_PIPELINE]->dsh.add_descriptor(i, 99, storage.get_buffer_by_name("uniform_buffer_" + std::to_string(i)));
		pipeline_data[UPDATE_HASH_GRID_PIPELINE]->dsh.add_descriptor(i, 0, storage.get_buffer(buffers[ANTS_BUFFER]));
		pipeline_data[UPDATE_HASH_GRID_PIPELINE]->dsh.add_descriptor(i, 1, storage.get_buffer_by_name("hash_grid_buffer"));
		pipeline_data[UPDATE_HASH_GRID_PIPELINE]->dsh.add_descriptor(i, 99, storage.get_buffer_by_name("uniform_buffer_" + std::to_string(i)));
		pipeline_data[CLEAR_PIPELINE]->dsh.add_descriptor(i, 0, storage.get_buffer(buffers[ANTS_BUFFER]));
		pipeline_data[RENDER_PIPELINE]->dsh.add_descriptor(i, 0, storage.get_buffer(buffers[ANTS_BUFFER]));
		pipeline_data[RENDER_PIPELINE]->dsh.add_descriptor(i, 99, storage.get_buffer_by_name("uniform_buffer_" + std::to_string(i)));
	}
	for (std::unique_ptr<PipelineData>& pipeline_datum : pipeline_data) pipeline_datum->dsh.construct();
}
