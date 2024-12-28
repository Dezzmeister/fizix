#include "controllers/mode.h"

mode_controller::mode_controller(
	event_buses &_buses,
	fcad_event_bus &_events
) :
	event_listener<set_mode_event>(&_events),
	event_listener<program_start_event>(&_buses.lifecycle),
	events(_events)
{
	event_listener<set_mode_event>::subscribe();
	event_listener<program_start_event>::subscribe();
}

int mode_controller::handle(set_mode_event &event) {
	edit_mode old_mode = curr_mode;
	curr_mode = event.new_mode;

	mode_switch_event mode_event(old_mode, curr_mode, event.hint);
	return events.fire(mode_event);
}

int mode_controller::handle(program_start_event&) {
	mode_switch_event mode_event(edit_mode::Normal, edit_mode::Normal);
	return events.fire(mode_event);
}

template <>
std::string traits::to_string(const edit_mode &mode, size_t) {
	switch (mode) {
		case edit_mode::Normal:
			return "NORMAL";
		case edit_mode::Command:
			return "COMMAND";
		case edit_mode::Select:
			return "SELECT";
		default:
			std::unreachable();
	}
}