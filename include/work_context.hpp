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
	S_RENDER_FINISHED = 1,
	S_ANTS_STEP_FINISHED = 2,
	S_FRAME_TO_FRAME = 3,
	SEMAPHORE_COUNT
};

enum FenceNames
{
	F_RENDER_FINISHED = 0,
	F_COMPUTE_FINISHED = 1,
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
	const vkte::VulkanMainContext& vmc;
	vkte::VulkanCommandContext& vcc;
	vkte::Storage storage;
	vkte::Swapchain swapchain;
	Ants ants;
	HashGrid hash_grid;
	UI ui;
	std::vector<vkte::Synchronization> syncs;
	std::vector<vkte::DeviceTimer> device_timers;
	std::vector<Timer<>> timers;

	void render_ui(vk::CommandBuffer& cb, AppState& app_state);
	void render(uint32_t image_idx, AppState& app_state);
};
