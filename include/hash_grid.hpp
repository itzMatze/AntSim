#pragma once

#include "vec2.hpp"

#include "app_state.hpp"
#include "vkte/descriptor_set_handler.hpp"
#include "vkte/pipeline.hpp"
#include "vkte/storage.hpp"

struct HashGridCellData
{
	glm::ivec2 index;
	int food_amount = 0u;
	float pheromone_lifetime = 0.0f;
	float pheromone_distance = 0.0;
	uint32_t state_bits = 0u;
};

class HashGrid
{
public:
	HashGrid(const vkte::VulkanMainContext& vmc, vkte::Storage& storage);
	void setup_storage(AppState& app_state);
	void construct(const vkte::RenderPass& render_pass, AppState& app_state, uint32_t frames_in_flight);
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
		vkte::Pipeline pipeline;
		vkte::DescriptorSetHandler dsh;
	};

	const vkte::VulkanMainContext& vmc;
	vkte::Storage& storage;
	std::array<int32_t, BUFFER_COUNT> buffers;
	std::array<std::unique_ptr<PipelineData>, PIPELINE_COUNT> pipeline_data;

	struct StepPushConstants
	{
		glm::vec2 food_pos;
		int32_t food_amount;
	} spc;

	void create_pipelines(const vkte::RenderPass& render_pass, const AppState& app_state);
	void create_descriptor_set(uint32_t frames_in_flight);
	void clear_storage_indices();
};
