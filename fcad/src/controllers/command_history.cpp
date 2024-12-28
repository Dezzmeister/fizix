#include <filesystem>
#include "controllers/command_history.h"

void command_history_controller::add_command(const std::wstring &command) {
	commands.push_back(command);
}

void command_history_controller::write_replay_file(std::wofstream &wfs) const {
	for (const std::wstring &command : commands) {
		wfs << command << "\n";
	}
}