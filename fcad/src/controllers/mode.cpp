#include "controllers/mode.h"

mode_controller::mode_controller(
	event_buses &_buses,
	fcad_event_bus &_events
) :
	event_listener<program_start_event>(&_buses.lifecycle),
	events(_events)
{
	event_listener<program_start_event>::subscribe();
}

void mode_controller::set_mode(edit_mode new_mode, const std::wstring &command_hint) {
	if (curr_mode == new_mode) {
		return;
	}

	edit_mode old_mode = curr_mode;

	mode_switch_event mode_event(old_mode, new_mode, command_hint);

	if (! events.fire(mode_event)) {
		curr_mode = new_mode;
	}
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