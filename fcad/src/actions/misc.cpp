#include "actions/misc.h"

start_command_impl::start_command_impl(fcad_event_bus &_events) :
	events(_events) {}

void start_command_impl::on_accept(char) {
	set_mode_event event(edit_mode::Command, L":");
	events.fire(event);
}