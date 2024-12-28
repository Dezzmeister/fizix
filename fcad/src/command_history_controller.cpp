#include <filesystem>
#include <fstream>
#include "command_history_controller.h"

command_history_controller::command_history_controller(fcad_event_bus &_events) :
	event_listener<new_replay_file_event>(&_events)
{
	event_listener<new_replay_file_event>::subscribe();
}

void command_history_controller::add_command(const std::wstring &command) {
	commands.push_back(command);
}

int command_history_controller::handle(new_replay_file_event &event) {
	std::filesystem::path file_path = std::filesystem::absolute(std::filesystem::path(event.path));

	logger::info("Saving replay file to " + file_path.string());

	std::wofstream wfs(file_path);

	for (const std::wstring &command : commands) {
		wfs << command << "\n";
	}

	return 0;
}