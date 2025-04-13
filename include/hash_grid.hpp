#pragma once

#include "vec2.hpp"

#include "app_state.hpp"
#include "vk/descriptor_set_handler.hpp"
#include "vk/pipeline.hpp"
#include "vk/storage.hpp"
#include <limits>

struct HashGridCellData
{
	glm::ivec2 index;
	uint active_flags = 0u;
	uint food_amount = 0u;
	float distance_to_nest = std::numeric_limits<float>::max();
	float distance_to_nest_age = 0.0f;
	float distance_to_food = std::numeric_limits<float>::max();
	float distance_to_food_age = 0.0f;
};

namespace ve
{
class HashGrid
{
public:
	HashGrid(const VulkanMainContext& vmc, Storage& storage);
	void setup_storage(AppState& app_state);
	void construct(const RenderPass& render_pass, AppState& app_state);
	void destruct();
	void clear(vk::CommandBuffer& cb, AppState& app_state);
	void compute(vk::CommandBuffer& cb, AppState& app_state);
	void render(vk::CommandBuffer& cb, AppState& app_state, const vk::Framebuffer& framebuffer, const vk::RenderPass& render_pass);

private:
	enum Buffers
	{
		HASH_GRID_BUFFER = 0,
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

	struct StepPushConstants
	{
		glm::vec2 food_pos;
		uint32_t food_amount;
	} spc;

	struct RenderPushConstants
	{
		glm::vec2 range_min;
		glm::vec2 range_max;
	} rpc;

	void create_pipelines(const RenderPass& render_pass, const AppState& app_state);
	void create_descriptor_set();
	void clear_storage_indices();
};
} // namespace ve
