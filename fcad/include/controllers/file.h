#pragma once
#include <filesystem>
#include <stack>
#include "edit_history.h"
#include "fcad_events.h"
#include "geometry.h"

class file_controller :
	traits::pinned<file_controller>,
	public event_listener<fcad_start_event>
{
public:
	file_controller(fcad_event_bus &_events);

	void write_file(const std::wstring &path);
	void read_file(const std::wstring &path, bool make_last_touched = true);

	int handle(fcad_start_event &event) override;

private:
	fcad_event_bus &events;
	edit_history_controller * edit_history{};
	geometry_controller * geom{};
	platform_bridge * platform{};
	std::optional<std::filesystem::path> last_touched_file{};

	void write_replay_file(const std::filesystem::path &path);
	void read_replay_file(const std::filesystem::path &path);
	void write_stl_file(const std::filesystem::path &path);
	void read_stl_file(const std::filesystem::path &path);

	void read_file(const std::filesystem::path &path, bool make_last_touched = true);
};