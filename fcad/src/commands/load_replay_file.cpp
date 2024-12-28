#include "commands.h"

void load_replay_file_command_impl::on_submit(const std::wstring &args) {
	load_replay_file_event event(args);
	events.fire(event);
}