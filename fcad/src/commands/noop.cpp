#include "commands.h"
#include "helpers.h"

noop_command_impl::noop_command_impl(fcad_event_bus &_events) :
	event_listener<fcad_start_event>(&_events),
	events(_events)
{
	event_listener<fcad_start_event>::subscribe();
}

void noop_command_impl::on_cancel(const std::wstring&) {}
void noop_command_impl::on_input(const std::wstring&) {}
void noop_command_impl::on_submit(const std::wstring&) {}

int noop_command_impl::handle(fcad_start_event &event) {
	history = &event.edit_history;
	files = &event.fc;
	geom = &event.gc;
	camera = &event.camera;
	platform = &event.platform;
	clipboard = &event.clipboard;
	params = &event.params;
	prefs = &event.prefs;

	return 0;
}

void noop_command_impl::write_help_text(std::ostream &os) const {
	write_help_rtf_row(os, "", "Does nothing.");
}

void noop_command_impl::set_output(const std::wstring &str) const {
	if (prefs->log_cmd_output()) {
		logger::info(L"Command output: " + str);
	}

	platform->set_cue_text(str);
}