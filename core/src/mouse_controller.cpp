#include <vector>
#include "controllers.h"

namespace {
	std::vector<mouse_controller *> mouse_controllers{};
	bool scroll_callback_bound = false;

	// Unfortunately, scroll input is only accessible through a callback, so we need
	// to do something a little ugly here. I don't know why there would ever be more
	// than one mouse controller, but in case there are, they won't interfere with
	// each other.
	void scroll_callback(GLFWwindow *, double x_offset, double y_offset) {
		for (mouse_controller * controller : mouse_controllers) {
			controller->set_scroll_offset((float)x_offset, (float)y_offset);
		}
	}
}

mouse_controller::mouse_controller(
	event_buses &_buses,
	std::vector<uint8_t> _watched_buttons,
	short _mouse_unlock_key
) :
	event_listener<pre_render_pass_event>(&_buses.render, -20),
	event_listener<program_start_event>(&_buses.lifecycle),
	event_listener<keydown_event>(&_buses.input),
	buses(_buses),
	watched_buttons(_watched_buttons),
	mouse_unlock_key(_mouse_unlock_key)
{
	event_listener<pre_render_pass_event>::subscribe();
	event_listener<program_start_event>::subscribe();
	event_listener<keydown_event>::subscribe();

	mouse_controllers.push_back(this);

	if (std::find(std::begin(watched_buttons), std::end(watched_buttons), GLFW_MOUSE_BUTTON_LEFT) == watched_buttons.end()) {
		watched_buttons.push_back(GLFW_MOUSE_BUTTON_LEFT);
	}
}

mouse_controller::~mouse_controller() {
	std::erase(mouse_controllers, this);
}

int mouse_controller::handle(pre_render_pass_event &event) {
	for (uint8_t button : watched_buttons) {
		const int is_pressed = glfwGetMouseButton(event.window, button);
		const int was_pressed = buttons[button];

		if (is_pressed == was_pressed) {
			continue;
		}

		buttons[button] = is_pressed;

		if (is_pressed) {
			mousedown_event mouse_event(button, is_mouse_locked);
			buses.input.fire(mouse_event);
		} else {
			mouseup_event mouse_event(button, is_mouse_locked);
			buses.input.fire(mouse_event);
		}
	}

	if (scroll_offset.x != 0.0f || scroll_offset.y != 0.0f) {
		mouse_scroll_event mouse_event(scroll_offset, is_mouse_locked);
		buses.input.fire(mouse_event);
		scroll_offset = glm::vec2(0.0f);
	}

	if (! is_mouse_locked && buttons[GLFW_MOUSE_BUTTON_LEFT]) {
		glfwSetInputMode(event.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		mouse_lock_event lock_event(event.window);
		buses.input.fire(lock_event);

		is_mouse_locked = true;
		mouse_locked_window = event.window;
	}

	return 0;
}

int mouse_controller::handle(program_start_event &event) {
	glfwSetInputMode(event.window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);

	if (! scroll_callback_bound) {
		glfwSetScrollCallback(event.window, scroll_callback);
	}

	return 0;
}

int mouse_controller::handle(keydown_event &event) {
	if (is_mouse_locked && event.key == mouse_unlock_key) {
		glfwSetInputMode(mouse_locked_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

		mouse_unlock_event unlock_event(mouse_locked_window);
		buses.input.fire(unlock_event);

		is_mouse_locked = false;
		mouse_locked_window = nullptr;
	}

	return 0;
}

void mouse_controller::set_scroll_offset(float x, float y) {
	scroll_offset.x = x;
	scroll_offset.y = y;
}