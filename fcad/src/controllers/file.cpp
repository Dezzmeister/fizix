#include "controllers/file.h"
#include "helpers.h"
#include "parsing.h"

file_controller::file_controller(fcad_event_bus &_events) :
	event_listener<fcad_start_event>(&_events),
	events(_events)
{
	event_listener<fcad_start_event>::subscribe();
}

int file_controller::handle(fcad_start_event &event) {
	edit_history = &event.edit_history;
	geom = &event.gc;

	return 0;
}

void file_controller::write_replay_file(const std::wstring &path_str) {
	std::wstring trimmed_path = trim(path_str);
	std::optional<std::filesystem::path> path = active_file;

	if (! trimmed_path.empty()) {
		path = std::filesystem::absolute(trimmed_path);
	}

	if (! path) {
		return;
	}

	write_replay_file_event event(path->wstring());
	if (events.fire(event)) {
		logger::info("Cancelled writing replay file to " + path->string());

		return;
	}

	logger::info("Writing replay file to " + path->string());

	std::wofstream wfs(*path);
	edit_history->write_replay_file(wfs);
	active_file = path;
}

void file_controller::read_replay_file(const std::wstring &path_str) {
	std::wstring trimmed_path = trim(path_str);
	std::optional<std::filesystem::path> path = active_file;

	if (! trimmed_path.empty()) {
		path = std::filesystem::absolute(trimmed_path);
	}

	if (! path) {
		return;
	}

	read_replay_file_event event(path->wstring());
	if (events.fire(event)) {
		logger::info("Cancelled reading replay file from " + path->string());

		return;
	}

	logger::info("Reading replay file from " + path->string());

	std::wifstream wfs(*path);
	parsing::parser_state state(wfs);

	if (! wfs.is_open() || state.eof()) {
		logger::error("Unable to open replay file");
		return;
	}

	geom->reset();

	std::wstring line = parse_line(state);

	while (! line.empty()) {
		command_submit_event command_event(line);
		events.fire(command_event);

		line = parse_line(state);
	}

	active_file = path;
}