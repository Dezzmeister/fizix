#include "controllers.h"

screen_controller::screen_controller(event_buses &_buses) :
	event_listener<pre_render_pass_event>(&_buses.render, -20),
	event_listener<post_processing_event>(&_buses.render, -20),
	event_listener<program_start_event>(&_buses.lifecycle, -100),
	buses(_buses),
	screen_width(-1),
	screen_height(-1)
{
	event_listener<pre_render_pass_event>::subscribe();
	event_listener<post_processing_event>::subscribe();
	event_listener<program_start_event>::subscribe();
}

int screen_controller::handle(pre_render_pass_event &event) {
	int curr_width;
	int curr_height;

	glfwGetWindowSize(event.window, &curr_width, &curr_height);

	if (! curr_width || ! curr_height) {
		return 0;
	}

	if (curr_width != screen_width || curr_height != screen_height) {
		if (screen_width != -1 || screen_height != -1) {
			screen_resize_event resize_event(screen_width, screen_height, curr_width, curr_height);

			buses.render.fire(resize_event);
		}

		screen_width = curr_width;
		screen_height = curr_height;
	}

	event.screen_width = screen_width;
	event.screen_height = screen_height;

	return 0;
}

int screen_controller::handle(post_processing_event &event) {
	event.screen_width = screen_width;
	event.screen_height = screen_height;

	return 0;
}

int screen_controller::handle(program_start_event &event) {
	glfwGetWindowSize(event.window, &screen_width, &screen_height);

	event.screen_width = screen_width;
	event.screen_height = screen_height;

	return 0;
}