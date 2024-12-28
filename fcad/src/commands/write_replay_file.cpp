#include "commands.h"
#include "helpers.h"

void write_replay_file_command_impl::on_submit(const std::wstring &args) {
	std::wstring path_from_args = trim(args);
	std::optional<std::wstring> path = active_file;

	if (! path_from_args.empty()) {
		path = path_from_args;
		active_file = path_from_args;
	}

	if (! path) {
		return;
	}

	new_replay_file_event event(*path);
	events.fire(event);
}