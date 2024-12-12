#include "controllers.h"

mouse_controller::mouse_controller(
	event_buses &_buses,
	std::vector<uint8_t> _watched_buttons,
	short _mouse_unlock_key
) :
	event_listener<pre_render_pass_event>(&_buses.render, -20),
	event_listener<keydown_event>(&_buses.input),
	buses(_buses),
	watched_buttons(_watched_buttons),
	mouse_unlock_key(_mouse_unlock_key)
{
	event_listener<pre_render_pass_event>::subscribe();
	event_listener<keydown_event>::subscribe();

	if (std::find(std::begin(watched_buttons), std::end(watched_buttons), MOUSE_LEFT) == watched_buttons.end()) {
		watched_buttons.push_back(MOUSE_LEFT);
	}
}

int mouse_controller::handle(pre_render_pass_event &event) {
	platform::cursor mouse = event.window->get_cursor();

	for (uint8_t button : watched_buttons) {
		const bool is_pressed = event.window->is_mouse_btn_down(button);
		const bool was_pressed = (bool)buttons[button];

		if (is_pressed == was_pressed) {
			continue;
		}

		buttons[button] = is_pressed;

		if (is_pressed) {
			mousedown_event mouse_event(button, mouse.is_locked);
			buses.input.fire(mouse_event);
		} else {
			mouseup_event mouse_event(button, mouse.is_locked);
			buses.input.fire(mouse_event);
		}
	}

	if (mouse.delta_x || mouse.delta_y) {
		event.window->reset_cursor_deltas();

		mouse_move_event mouse_event(mouse.delta_x, mouse.delta_y);
		buses.input.fire(mouse_event);
	}

	if (mouse.delta_vscroll || mouse.delta_hscroll) {
		event.window->reset_cursor_deltas();

		mouse_scroll_event mouse_event(mouse.delta_vscroll, mouse.delta_hscroll, mouse.is_locked);
		buses.input.fire(mouse_event);
	}

	if (! mouse.is_locked && event.window->is_mouse_btn_down(MOUSE_LEFT)) {
		event.window->lock_cursor();

		mouse_lock_event lock_event(event.window);
		buses.input.fire(lock_event);

		mouse_locked_window = event.window;
	}

	return 0;
}

int mouse_controller::handle(keydown_event &event) {
	if (event.is_mouse_locked && event.key == mouse_unlock_key) {
		mouse_locked_window->unlock_cursor();

		mouse_unlock_event unlock_event(mouse_locked_window);
		buses.input.fire(unlock_event);

		mouse_locked_window = nullptr;
	}

	return 0;
}