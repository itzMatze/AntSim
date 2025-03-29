#pragma once
#include <cstdint>
#include "vk/device_timer.hpp"
#include "vec2.hpp"

struct AppState {
public:
	AppState() : current_frame(0), total_frames(0), total_time(1.0f/60.0f), vsync(true), show_ui(true)
	{
		for (int i = 0; i < device_timings.size(); i++) device_timings[i] = 0.0f;
	}

	uint32_t current_frame = 0;
	uint32_t total_frames = 0;
	std::array<float, ve::DeviceTimer::TIMER_COUNT> device_timings;
	float frame_time = 1.0 / 60.0;
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

