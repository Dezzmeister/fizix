#include "controllers/action.h"

action_controller::action_controller(
	fcad_event_bus &_events,
	action_group &_window_actions
) :
	event_listener<window_input_event>(&_events),
	window_actions(_window_actions)
{
	event_listener<window_input_event>::subscribe();
}

int action_controller::handle(window_input_event &event) {
	window_actions.test(event.c);

	return 0;
}