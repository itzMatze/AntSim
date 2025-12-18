#pragma once

#include <vector>

#include "ants.hpp"
#include "app_state.hpp"
#include "hash_grid.hpp"
#include "util/timer.hpp"
#include "vkte_window/ui.hpp"
#include "vkte/device_timer.hpp"
#include "vkte/storage.hpp"
#include "vkte/swapchain.hpp"
#include "vkte/vulkan_command_context.hpp"
#include "vkte/vulkan_main_context.hpp"
#include "vkte/synchronization.hpp"

enum SemaphoreNames
{
	S_IMAGE_AVAILABLE = 0,
	S_ANTS_STEP_FINISHED = 1,
	S_FRAME_TO_FRAME = 2,
	SEMAPHORE_COUNT
};

enum FenceNames
{
	F_RENDER_FINISHED = 0,
	FENCE_COUNT
};

// manages the orchestration of rendering and computing
class WorkContext
{
public:
	WorkContext(const vkte::VulkanMainContext& vmc, vkte::VulkanCommandContext& vcc);
	void construct(AppState& app_state);
	void destruct();
	void draw_frame(AppState& app_state);
	vk::Extent2D resize(bool vsync);

private:
	static constexpr uint32_t frames_in_flight = 2;

	struct UniformBufferData
	{
		glm::vec2 range_min;
		glm::vec2 range_max;
		uint32_t food_vis_code;
		uint32_t food_pheromone_vis_code;
		uint32_t nest_vis_code;
		uint32_t nest_pheromone_vis_code;
		uint32_t ant_wo_food_vis_code;
		uint32_t ant_wo_food_dir_vis_code;
		uint32_t ant_w_food_vis_code;
		uint32_t ant_w_food_dir_vis_code;
		uint32_t frame_idx;
		float frame_time;
		float total_time;
	} uniform_buffer_data;

	const vkte::VulkanMainContext& vmc;
	vkte::VulkanCommandContext& vcc;
	vkte::Storage storage;
	vkte::Swapchain swapchain;
	Ants ants;
	HashGrid hash_grid;
	vkte::UI ui;
	std::vector<vkte::Synchronization> syncs;
	vkte::Synchronization swapchain_sync;
	std::vector<vkte::DeviceTimer> device_timers;
	std::array<Timer<>, frames_in_flight> timers;
	std::array<uint32_t, frames_in_flight> uniform_buffers;

	void render_ui(vk::CommandBuffer& cb, AppState& app_state);
	void render(uint32_t image_idx, AppState& app_state);
};
