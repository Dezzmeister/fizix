#include "commands.h"
#include "helpers.h"

void write_replay_file_command_impl::on_submit(const std::wstring &args) {
	new_replay_file_event event(args);
	events.fire(event);
}