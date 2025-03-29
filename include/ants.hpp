#pragma once

#include "vec2.hpp"

#include "app_state.hpp"
#include "vk/descriptor_set_handler.hpp"
#include "vk/pipeline.hpp"
#include "vk/storage.hpp"

struct AntData
{
	glm::vec2 pos;
	glm::vec2 dir;
};

namespace ve
{
class Ants
{
public:
	Ants(const VulkanMainContext& vmc, Storage& storage);
	void setup_storage(AppState& app_state);
	void construct(const RenderPass& render_pass, AppState& app_state);
	void destruct();
	void clear(vk::CommandBuffer& cb, AppState& app_state);
	void compute(vk::CommandBuffer& cb, AppState& app_state);
	void render(vk::CommandBuffer& cb, AppState& app_state, const vk::Framebuffer& framebuffer, const vk::RenderPass& render_pass);

private:
	enum Buffers
	{
		ANTS_BUFFER_0 = 0,
		ANTS_BUFFER_1 = 1,
		BUFFER_COUNT
	};

	enum Pipelines
	{
		CLEAR_PIPELINE = 0,
		RENDER_PIPELINE = 1,
		STEP_PIPELINE = 2,
		PIPELINE_COUNT
	};

	struct PipelineData
	{
		Pipeline pipeline;
		DescriptorSetHandler dsh;
	};

	const VulkanMainContext& vmc;
	Storage& storage;
	std::array<int32_t, BUFFER_COUNT> buffers;
	std::array<std::unique_ptr<PipelineData>, PIPELINE_COUNT> pipeline_data;

	uint32_t ant_count = 100;

	struct PushConstants
	{
		uint32_t frame_idx;
	} pc;

	void create_pipelines(const RenderPass& render_pass, const AppState& app_state);
	void create_descriptor_set();
	void clear_storage_indices();
};
} // namespace ve
