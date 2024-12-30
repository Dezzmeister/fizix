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

void file_controller::write_file(const std::wstring &_path_str) {
	std::wstring trimmed_path = trim(_path_str);
	std::optional<std::filesystem::path> path = active_file;

	if (! trimmed_path.empty()) {
		path = std::filesystem::absolute(trimmed_path);
	}

	if (! path) {
		return;
	}

	std::wstring path_str = path->wstring();

	if (path_str.ends_with(L".stl")) {
		write_stl_file(*path);
	} else {
		write_replay_file(*path);
	}

	active_file = path;
}

void file_controller::read_file(const std::wstring &path_str) {
	std::wstring trimmed_path = trim(path_str);
	std::optional<std::filesystem::path> path = active_file;

	if (! trimmed_path.empty()) {
		path = std::filesystem::absolute(trimmed_path);
	}

	if (! path) {
		return;
	}

	// TODO: STL
	read_replay_file(*path);
	active_file = path;
}

void file_controller::write_replay_file(const std::filesystem::path &path) {
	write_replay_file_event event(path);
	if (events.fire(event)) {
		logger::info("Cancelled writing replay file to " + path.string());

		return;
	}

	logger::info("Writing replay file to " + path.string());

	std::wofstream wfs(path);

	if (! wfs.is_open() || wfs.bad()) {
		logger::error("Unable to open " + path.string() + " for writing");
		return;
	}

	edit_history->write_replay_file(wfs);
}

void file_controller::read_replay_file(const std::filesystem::path &path) {
	read_replay_file_event event(path);
	if (events.fire(event)) {
		logger::info("Cancelled reading replay file from " + path.string());

		return;
	}

	logger::info("Reading replay file from " + path.string());

	std::wifstream wfs(path);
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
}

void file_controller::write_stl_file(const std::filesystem::path &path) {
	write_stl_file_event event(path);
	if (events.fire(event)) {
		logger::info("Cancelled writing STL file to " + path.string());

		return;
	}

	logger::info("Writing STL file to " + path.string());

	std::wofstream wfs(path);

	if (! wfs.is_open() || wfs.bad()) {
		logger::error("Failed to open " + path.string() + " for writing");
		return;
	}

	std::wstring path_wstr = path.wstring();
	std::wstring solid_name = path_wstr.substr(0, path_wstr.size() - std::wstring(L".stl").size());

	wfs << L"solid " << solid_name << std::endl;
	wfs << std::setprecision(6);

	for (const triangle &tri : geom->faces()) {
		// TODO: Do STL floats always need the sign and exponent?
		wfs << L"facet normal ";
		wfs << tri.normal.x << L" ";
		wfs << tri.normal.y << L" ";
		wfs << tri.normal.z << std::endl;

		wfs << L"\touter loop" << std::endl;
		wfs << L"\t\tvertex ";
		wfs << tri.v1.x << L" ";
		wfs << tri.v1.y << L" ";
		wfs << tri.v1.z << std::endl;
		wfs << L"\t\tvertex ";
		wfs << tri.v2.x << L" ";
		wfs << tri.v2.y << L" ";
		wfs << tri.v2.z << std::endl;
		wfs << L"\t\tvertex ";
		wfs << tri.v3.x << L" ";
		wfs << tri.v3.y << L" ";
		wfs << tri.v3.z << std::endl;

		wfs << L"\tendloop" << std::endl;
		wfs << L"endfacet" << std::endl;
	}

	wfs << "endsolid " << solid_name;
}