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

void action_controller::write_help_text(std::ostream &os) const {
	window_actions.write_help_text(os);
}

int action_controller::handle(window_input_event &event) {
	window_actions.test(event.c);

	return 0;
}