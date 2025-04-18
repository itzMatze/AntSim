#pragma once

#include <vector>

#include "ants.hpp"
#include "app_state.hpp"
#include "hash_grid.hpp"
#include "ui.hpp"
#include "util/timer.hpp"
#include "vk/device_timer.hpp"
#include "vk/storage.hpp"
#include "vk/swapchain.hpp"
#include "vk/vulkan_command_context.hpp"
#include "vk/vulkan_main_context.hpp"
#include "vk/synchronization.hpp"

namespace ve
{
// manages the orchestration of rendering and computing
class WorkContext
{
public:
	WorkContext(const VulkanMainContext& vmc, VulkanCommandContext& vcc);
	void construct(AppState& app_state);
	void destruct();
	void draw_frame(AppState& app_state);
	vk::Extent2D resize(bool vsync);

private:
	const VulkanMainContext& vmc;
	VulkanCommandContext& vcc;
	Storage storage;
	Swapchain swapchain;
	Ants ants;
	HashGrid hash_grid;
	UI ui;
	std::vector<Synchronization> syncs;
	std::vector<DeviceTimer> device_timers;
	std::vector<Timer<>> timers;

	void render(uint32_t image_idx, AppState& app_state);
};
} // namespace ve
