#include "actions/misc.h"

start_command_impl::start_command_impl(fcad_event_bus &_events) :
	events(_events) {}

void start_command_impl::on_accept(char) {
	set_mode_event event(edit_mode::Command, L":");
	events.fire(event);
}

undo_edit_impl::undo_edit_impl(fcad_event_bus &_events) :
	event_listener<fcad_start_event>(&_events)
{
	event_listener<fcad_start_event>::subscribe();
}

int undo_edit_impl::handle(fcad_start_event &event) {
	history = &event.edit_history;

	return 0;
}

void undo_edit_impl::on_accept(char) {
	history->undo();
}

redo_edit_impl::redo_edit_impl(fcad_event_bus &_events) :
	event_listener<fcad_start_event>(&_events)
{
	event_listener<fcad_start_event>::subscribe();
}

int redo_edit_impl::handle(fcad_start_event &event) {
	history = &event.edit_history;

	return 0;
}

void redo_edit_impl::on_accept(char) {
	history->redo();
}