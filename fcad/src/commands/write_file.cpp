#include "commands.h"
#include "helpers.h"

void write_file_command_impl::on_submit(const std::wstring &args) {
	files->write_file(args);
}

void write_file_command_impl::write_help_text(std::ostream &os) const {
	write_help_rtf_row(os, ":w [filename]",
		"Writes a file to disk. For now, only replay files and STLs are supported. "
		"If {\\b filename} is not provided, the last active file (whichever "
		"was last read from or written to) will be written to. If {\\b filename} ends "
		"with \".stl\", the file written will be an STL file instead of a replay file."
	);
}
