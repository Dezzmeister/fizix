#include "commands.h"
#include "helpers.h"

void read_file_command_impl::on_submit(const std::wstring &args) {
	files->read_file(args);
}

void read_file_command_impl::write_help_text(std::ostream &os) const {
	write_help_rtf_row(os, ":r [filename]",
		"Reads a file and loads it in the editor. For now, only "
		"replay files are supported. If {\\b filename} is not provided, "
		"the last active file (whichever was read from or written to) will "
		"be read."
	);
}