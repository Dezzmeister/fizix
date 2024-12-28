#pragma once
#include "fcad_events.h"

class command_history_controller :
	public event_listener<new_replay_file_event>
{
public:
	command_history_controller(fcad_event_bus &_events);

	void add_command(const std::wstring &command);

	int handle(new_replay_file_event &event) override;

private:
	// TODO: Max command history, restorable states
	std::vector<std::wstring> commands{};
};
