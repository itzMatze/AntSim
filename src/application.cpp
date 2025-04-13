#include "application.hpp"

#include "SDL3/SDL_events.h"
#include "event_handler.hpp"
#include "work_context.hpp"
#include "util/timer.hpp"
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_video.h>

struct GPUContext
{
	GPUContext(AppState& app_state) : vmc(), vcc(vmc), wc(vmc, vcc)
	{
		vmc.construct(app_state.get_window_extent().width, app_state.get_window_extent().height);
		vcc.construct();
		wc.construct(app_state);
	}

	~GPUContext()
	{
		wc.destruct();
		vcc.destruct();
		vmc.destruct();
	}
	ve::VulkanMainContext vmc;
	ve::VulkanCommandContext vcc;
	ve::WorkContext wc;
};

void dispatch_pressed_keys(EventHandler& event_handler, AppState& app_state, Window& window)
{
	if (event_handler.is_key_released(Key::G))
	{
		app_state.show_ui = !app_state.show_ui;
		event_handler.set_released_key(Key::G, false);
	}
	if (event_handler.is_key_pressed(Key::MouseLeft))
	{
		if (!SDL_GetWindowRelativeMouseMode(window.get()))
		{
			event_handler.mouse_motion = glm::vec2(0.0f);
			SDL_SetWindowRelativeMouseMode(window.get(), true);
		}
		app_state.visible_range_min -= event_handler.mouse_motion * 0.1f;
		app_state.visible_range_max -= event_handler.mouse_motion * 0.1f;
		event_handler.mouse_motion = glm::vec2(0.0f);
	}
	if (event_handler.is_key_released(Key::MouseLeft))
	{
		SDL_SetWindowRelativeMouseMode(window.get(), false);
		SDL_WarpMouseInWindow(window.get(), app_state.get_window_extent().width / 2.0f, app_state.get_window_extent().height / 2.0f);
		event_handler.set_released_key(Key::MouseLeft, false);
	}
	if (std::abs(event_handler.mouse_wheel_motion.y) > 0.001f)
	{
		const glm::vec2 center = (app_state.visible_range_min + app_state.visible_range_max) / glm::vec2(2.0f, 2.0f);
		// the length of the displayed range is always the same for both axis
		glm::vec2 centered_range = glm::vec2(app_state.visible_range_min.x, app_state.visible_range_max.x) - glm::vec2(center.x);
		if (event_handler.mouse_wheel_motion.y < 0.0f) centered_range *= 1.2;
		else centered_range /= 1.2;
		app_state.visible_range_min = glm::vec2(centered_range.x) + center;
		app_state.visible_range_max = glm::vec2(centered_range.y) + center;
		event_handler.mouse_wheel_motion.y = 0.0f;
	}
}

int run_application(glm::ivec2 window_resolution)
{
	AppState app_state;
	app_state.set_render_extent(vk::Extent2D(window_resolution.x, window_resolution.y));
	GPUContext gpu_context(app_state);
	EventHandler event_handler;

	bool quit = false;
	Timer rendering_timer;
	SDL_Event e;
	while (!quit)
	{
		dispatch_pressed_keys(event_handler, app_state, *gpu_context.vmc.window);
		try
		{
			gpu_context.wc.draw_frame(app_state);
		}
		catch (const vk::OutOfDateKHRError e)
		{
			gpu_context.vmc.logical_device.get().waitIdle();
			app_state.current_frame = 0;
			app_state.total_frames = 0;
			app_state.set_window_extent(gpu_context.wc.resize(app_state.vsync));
		}
		while (SDL_PollEvent(&e))
		{
			quit |= e.window.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED;
			event_handler.dispatch_event(e);
		}
	}
	return 0;
}
