#include "commands.h"

void help_command_impl::on_submit(const std::wstring&) {
	platform->set_cue_text(L"TODO");
}