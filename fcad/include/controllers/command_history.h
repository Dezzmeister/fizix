#pragma once
#include <fstream>
#include "fcad_events.h"

class command_history_controller :
	traits::pinned<command_history_controller>
{
public:
	void add_command(const std::wstring &command);
	void write_replay_file(std::wofstream &wfs) const;

private:
	// TODO: Max command history, restorable states
	std::vector<std::wstring> commands{};
};
