#include "commands.h"
#include "helpers.h"

void write_file_command_impl::on_submit(const std::wstring &args) {
	files->write_replay_file(args);
}