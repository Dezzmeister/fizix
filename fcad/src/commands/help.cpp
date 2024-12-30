#include "commands.h"
#include "controllers/action.h"
#include "controllers/command.h"
#include "helpers.h"

const std::string help_text_intro =
R"({\rtf1\ansi\deff0
{\fonttbl
{\f0\froman Times New Roman;}
{\f1\fmodern Lucida Console;}
}
\deflang1033\plain\f0\fs20\hyphauto
{\pard\qc\fs48
FCAD
\par}
{\pard\fs20\li50\ri50
FCAD supports actions and commands. Actions are executed by entering sequences of characters
 in the main window, and commands are executed by entering a {\b :}, typing in a command and
 optional arguments, and pressing {\b ENTER}. Some actions are just shortcuts for commands. These
 actions fill the command bar with the name of a command and give focus to the command bar so that
 the user can finish entering the command. FCAD also supports basic controls for manipulating the
 camera. These are keys (not characters) that can be held to move the camera.
\par}
)";

int help_command_impl::handle(fcad_start_event &event) {
	noop_command_impl::handle(event);
	std::ostringstream oss{};

	oss << help_text_intro;

	write_help_rtf_header(oss, "Controls");
	write_help_rtf_row(oss, "HJKL",
		"Rotates the camera about the target. The target is at the origin by default."
	);
	write_help_rtf_row(oss, "CTRL + HJKL", "Moves the target.");
	write_help_rtf_row(oss, "IO",
		"Moves the camera in and out (towards and away from the target)."
	);
	write_help_rtf_row(oss, "CTRL + IO",
		"Moves the camera {\\b and the target} in and out."
	);
	write_help_rtf_row(oss, "X + HL", "Rotates the camera about the X axis.");
	write_help_rtf_row(oss, "Y + HL", "Rotates the camera about the Y axis.");
	write_help_rtf_row(oss, "Z + HL", "Rotates the camera about the Z axis.");

	write_help_rtf_header(oss, "Actions");
	event.actions.write_help_text(oss);

	write_help_rtf_header(oss, "Commands");
	event.commands.write_help_text(oss);

	oss << "}";

	help_text = oss.str();

	return 0;
}

void help_command_impl::on_submit(const std::wstring&) {
	platform->create_help_dialog(help_text);
}

void help_command_impl::write_help_text(std::ostream &os) const {
	write_help_rtf_row(os, ":h", "Displays this help text.");
}