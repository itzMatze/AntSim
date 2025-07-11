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
	glm::vec2 target;
	float distance_to_poi;
	float pheromone_emit_scale;
	uint32_t state_bits;
	uint32_t pad0;
};

struct NestData
{
	glm::vec2 pos;
	int32_t level;
	uint32_t food_amount;
};

class Ants
{
public:
	Ants(const vkte::VulkanMainContext& vmc, vkte::Storage& storage);
	void setup_storage(AppState& app_state);
	void construct(const vkte::RenderPass& render_pass, AppState& app_state, uint32_t frames_in_flight);
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

	void create_pipelines(const vkte::RenderPass& render_pass, const AppState& app_state);
	void create_descriptor_set(uint32_t frames_in_flight);
	void clear_storage_indices();
};
