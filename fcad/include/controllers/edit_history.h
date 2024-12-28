#pragma once
#include <fstream>
#include "fcad_events.h"
#include "geometry.h"

class edit_history_controller :
	traits::pinned<edit_history_controller>,
	event_listener<fcad_start_event>
{
public:
	edit_history_controller(fcad_event_bus &_events);

	void add_command(const std::wstring &command);
	void write_replay_file(std::wofstream &wfs) const;
	void undo();
	void redo();
	void clear();

	int handle(fcad_start_event &event) override;

private:
	fcad_event_bus &events;
	geometry_controller * geom{};
	// TODO: Max command history, restorable states
	std::vector<std::wstring> commands{};
	size_t curr_pos{};
	bool is_history_locked{};
};
