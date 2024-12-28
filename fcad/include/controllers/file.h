#pragma once
#include <filesystem>
#include "command_history.h"
#include "fcad_events.h"
#include "geometry.h"

class file_controller :
	traits::pinned<file_controller>,
	public event_listener<fcad_start_event>,
	public event_listener<new_replay_file_event>,
	public event_listener<load_replay_file_event>
{
public:
	file_controller(fcad_event_bus &_events);

	int handle(fcad_start_event &event) override;
	int handle(new_replay_file_event &event) override;
	int handle(load_replay_file_event &event) override;

private:
	fcad_event_bus &events;
	command_history_controller * command_history{};
	geometry_controller * geom{};
	std::optional<std::filesystem::path> active_file{};
};