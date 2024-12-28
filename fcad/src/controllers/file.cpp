#include "controllers/file.h"
#include "helpers.h"
#include "parsing.h"

file_controller::file_controller(fcad_event_bus &_events) :
	event_listener<fcad_start_event>(&_events),
	event_listener<new_replay_file_event>(&_events),
	event_listener<load_replay_file_event>(&_events),
	events(_events)
{
	event_listener<fcad_start_event>::subscribe();
	event_listener<new_replay_file_event>::subscribe();
	event_listener<load_replay_file_event>::subscribe();
}

int file_controller::handle(fcad_start_event &event) {
	command_history = &event.command_history;
	geom = &event.gc;

	return 0;
}

int file_controller::handle(new_replay_file_event &event) {
	std::wstring trimmed_path = trim(event.path);
	std::optional<std::filesystem::path> path = active_file;

	if (! trimmed_path.empty()) {
		path = std::filesystem::absolute(trimmed_path);
	}

	if (! path) {
		return false;
	}

	logger::info("Saving replay file to " + path->string());

	std::wofstream wfs(*path);
	command_history->write_replay_file(wfs);
	active_file = path;

	return true;
}

int file_controller::handle(load_replay_file_event &event) {
	std::wstring trimmed_path = trim(event.path);
	std::optional<std::filesystem::path> path = active_file;

	if (! trimmed_path.empty()) {
		path = std::filesystem::absolute(trimmed_path);
	}

	if (! path) {
		return false;
	}

	logger::info("Loading replay file from " + path->string());

	std::wifstream wfs(*path);
	parsing::parser_state state(wfs);

	geom->reset();

	std::wstring line = parse_line(state);

	while (! line.empty()) {
		command_submit_event command_event(line);
		events.fire(command_event);

		line = parse_line(state);
	}

	active_file = path;

	return 0;
}