#include "window_action_controller.h"

window_action_controller::window_action_controller(
	fcad_event_bus &_events,
	action_group &_window_actions
) :
	event_listener<window_input_event>(&_events),
	window_actions(_window_actions)
{
	event_listener<window_input_event>::subscribe();
}

int window_action_controller::handle(window_input_event &event) {
	window_actions.test(event.c);

	return 0;
}