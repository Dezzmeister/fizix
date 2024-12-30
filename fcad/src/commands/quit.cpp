#include <platform/platform.h>
#include "commands.h"
#include "helpers.h"

void quit_command_impl::on_submit(const std::wstring&) {
	// TODO: Quit in a platform independent way
	PostQuitMessage(0);
}

void quit_command_impl::write_help_text(std::ostream &os) const {
	write_help_rtf_row(os, ":q", "Quits FCAD without saving.");
}