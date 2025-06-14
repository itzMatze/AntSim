#pragma once
#include <cstdint>
#include <array>
#include <vulkan/vulkan.hpp>
#include "vec2.hpp"
#include "vec3.hpp"

enum TimerNames {
	TIMER_RENDERING_ALL = 0,
	TIMER_ANTS_STEP = 1,
	TIMER_HASH_GRID_STEP = 2,
	TIMER_COUNT
};

struct VisualizationCodeData
{
	// food with viridis
	uint32_t food = 1;
	glm::vec3 food_color = glm::vec3(0.0f, 0.0f, 0.0f);
	// food pheromones in dark yellow
	uint32_t food_pheromone = 5;
	glm::vec3 food_pheromone_color = glm::vec3(0.4f, 0.0f, 0.4f);
	// nest with plasma
	uint32_t nest = 2;
	glm::vec3 nest_color = glm::vec3(0.0f, 0.0f, 0.0f);
	// nest pheromones in dark blue
	uint32_t nest_pheromone = 5;
	glm::vec3 nest_pheromone_color = glm::vec3(0.0f, 0.0f, 0.4f);
};

struct AppState {
public:
	AppState() : current_frame(0), total_frames(0), total_time(1.0f/60.0f), vsync(true), show_ui(true)
	{
		for (int i = 0; i < device_timings.size(); i++) device_timings[i] = 0.0f;
	}

	VisualizationCodeData vis_code_data;
	glm::vec2 add_food_pos;
	uint32_t add_food_amount = 0;
	glm::vec2 visible_range_min = glm::vec2(-5.0, -5.0);
	glm::vec2 visible_range_max = glm::vec2(5.0, 5.0);
	const uint32_t ant_count = 1'00;
	const uint32_t hash_grid_capacity = 100'000;
	uint32_t current_frame = 0;
	uint32_t total_frames = 0;
	std::array<float, TIMER_COUNT> device_timings;
	float frame_time = 1.0 / 60.0;
	float frame_time_ema = 1.0 / 60.0;
	float total_time = 1.0 / 60.0;
	bool vsync = true;
	bool show_ui = true;

	vk::Extent2D get_render_extent() const { return render_extent; }
	vk::Extent2D get_window_extent() const { return window_extent; }
	void set_render_extent(vk::Extent2D extent)
	{
		render_extent = extent;
		aspect_ratio = float(render_extent.width) / float(render_extent.height);
		window_extent = vk::Extent2D(aspect_ratio * 1000, 1000);
	}
	void set_window_extent(vk::Extent2D extent)
	{
		window_extent = extent;
	}

private:
	vk::Extent2D render_extent = vk::Extent2D(1920, 1080);
	float aspect_ratio = float(render_extent.width) / float(render_extent.height);
	vk::Extent2D window_extent = vk::Extent2D(aspect_ratio * 1000, 1000);
};
