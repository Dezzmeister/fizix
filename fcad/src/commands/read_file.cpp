#include "commands.h"

void read_file_command_impl::on_submit(const std::wstring &args) {
	files->read_replay_file(args);
}