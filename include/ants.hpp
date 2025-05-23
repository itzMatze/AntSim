#pragma once

#include "vec2.hpp"

#include "app_state.hpp"
#include "vkte/descriptor_set_handler.hpp"
#include "vkte/pipeline.hpp"
#include "vkte/storage.hpp"

struct AntData
{
	glm::vec2 pos;
	glm::vec2 dir;
	float distance_to_poi;
	uint32_t state_bits;
};

struct NestData
{
	glm::vec2 pos;
	float radius;
};

class Ants
{
public:
	Ants(const vkte::VulkanMainContext& vmc, vkte::Storage& storage);
	void setup_storage(AppState& app_state);
	void construct(const vkte::RenderPass& render_pass, AppState& app_state);
	void destruct();
	void clear(vk::CommandBuffer& cb, AppState& app_state);
	void compute(vk::CommandBuffer& cb, AppState& app_state);
	void render(vk::CommandBuffer& cb, AppState& app_state, const vk::Framebuffer& framebuffer, const vk::RenderPass& render_pass);

private:
	enum Buffers
	{
		ANTS_BUFFER = 0,
		NEST_BUFFER = 1,
		BUFFER_COUNT
	};

	enum Pipelines
	{
		CLEAR_PIPELINE = 0,
		RENDER_PIPELINE = 1,
		STEP_PIPELINE = 2,
		UPDATE_HASH_GRID_PIPELINE = 3,
		PIPELINE_COUNT
	};

	struct PipelineData
	{
		vkte::Pipeline pipeline;
		vkte::DescriptorSetHandler dsh;
	};

	const vkte::VulkanMainContext& vmc;
	vkte::Storage& storage;
	std::array<int32_t, BUFFER_COUNT> buffers;
	std::array<std::unique_ptr<PipelineData>, PIPELINE_COUNT> pipeline_data;

	uint32_t point_size = 2;

	struct StepPushConstants
	{
		uint32_t frame_idx;
		float frame_time;
		float total_time;
	} spc;

	struct UpdateHashGridPushConstants
	{
		uint32_t frame_idx;
		float frame_time;
		float total_time;
		float pheromone_lifetime = 60.0f;
	} uhgpc;

	struct RenderPushConstants
	{
		glm::vec2 range_min;
		glm::vec2 range_max;
	} rpc;

	void create_pipelines(const vkte::RenderPass& render_pass, const AppState& app_state);
	void create_descriptor_set();
	void clear_storage_indices();
};
