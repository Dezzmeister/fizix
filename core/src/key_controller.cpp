#include "controllers.h"

key_controller::key_controller(event_buses &_buses, std::vector<short> _watched_keys) :
	event_listener<pre_render_pass_event>(&_buses.render, -20),
	event_listener<mouse_lock_event>(&_buses.input),
	event_listener<mouse_unlock_event>(&_buses.input),
	buses(_buses),
	watched_keys(_watched_keys)
{
	event_listener<pre_render_pass_event>::subscribe();
	event_listener<mouse_lock_event>::subscribe();
	event_listener<mouse_unlock_event>::subscribe();
}

int key_controller::handle(pre_render_pass_event &event) {
	for (const short key : watched_keys) {
		const int is_pressed = glfwGetKey(event.window, key) == GLFW_PRESS;
		const int was_pressed = keys[key];

		if (is_pressed == was_pressed) {
			continue;
		}

		keys[key] = is_pressed;

		if (is_pressed) {
			keydown_event key_event(key, is_mouse_locked);
			buses.input.fire(key_event);
		} else {
			keyup_event key_event(key, is_mouse_locked);
			buses.input.fire(key_event);
		}
	}

	return 0;
}

int key_controller::handle(mouse_lock_event&) {
	is_mouse_locked = true;

	return 0;
}

int key_controller::handle(mouse_unlock_event&) {
	is_mouse_locked = false;

	return 0;
}
