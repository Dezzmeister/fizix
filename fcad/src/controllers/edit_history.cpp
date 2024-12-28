#include <filesystem>
#include "controllers/edit_history.h"

edit_history_controller::edit_history_controller(
	fcad_event_bus &_events
) :
	event_listener<fcad_start_event>(&_events),
	events(_events)
{
	event_listener<fcad_start_event>::subscribe();
}

void edit_history_controller::add_command(const std::wstring &command) {
	if (is_history_locked) {
		return;
	}

	if (curr_pos + 1 < commands.size()) {
		commands.erase(std::begin(commands) + curr_pos, std::end(commands));
	}

	commands.push_back(command);
	curr_pos++;
}

void edit_history_controller::write_replay_file(std::wofstream &wfs) const {
	for (const std::wstring &command : commands) {
		wfs << command << "\n";
	}
}

void edit_history_controller::undo() {
	if (curr_pos == 0) {
		return;
	}

	// TODO: Be smarter about undos. Either make each command reversible, or keep
	// a base state with N subsequent edits
	geom->reset();

	curr_pos--;
	assert(curr_pos < commands.size());

	is_history_locked = true;

	for (size_t i = 0; i < curr_pos; i++) {
		command_submit_event event(commands[i]);
		events.fire(event);
	}

	is_history_locked = false;
}

void edit_history_controller::redo() {
	if (curr_pos == commands.size()) {
		return;
	}

	assert(curr_pos < commands.size());

	is_history_locked = true;

	command_submit_event event(commands[curr_pos]);
	events.fire(event);
	curr_pos++;

	is_history_locked = false;
}

void edit_history_controller::clear() {
	commands.clear();
	curr_pos = 0;
}

int edit_history_controller::handle(fcad_start_event &event) {
	geom = &event.gc;

	return 0;
}