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
{\pard\qc\fs48\f0
FCAD
\par}
{\pard\fs20\f0\li50\ri50
FCAD supports actions and commands. Actions are executed by entering sequences of characters
 in the main window, and commands are executed by entering a {\f1\b :}, typing in a command and
 optional arguments, and pressing {\f1\b ENTER}. Some actions are just shortcuts for commands. These
 actions fill the command bar with the name of a command and give focus to the command bar so that
 the user can finish entering the command. FCAD also supports basic controls for manipulating the
 camera. These are keys (not characters) that can be held to move the camera.
\par})";

const std::string commands_intro =
R"({\pard\fs20\f0\li50\ri50
Typing a '{\f1\b :}' while in Normal mode focuses the command bar and switches to Command mode.
 In this mode, you can enter a command (and arguments). When you press {\f1\b ENTER}, FCAD will attempt to
 execute the command.
\par}
{\pard\fs20\f0\li50\ri50
Some commands take arguments. In this table, required arguments are marked with angle brackets, and
 optional arguments are marked with square brackets. Some commands have arguments that accept more
 than one type of feature (e.g. vertex, edge, or face). For these arguments, the feature must be
 specified explicitly:\line
{\f1\fs14 v1}\tab\tab The vertex with index 1\line
{\f1\fs14 e(4 9)}\tab\tab The edge connecting vertex 4 and vertex 9\line
{\f1\fs14 f(8 2 0)}\tab\tab The face with vertices 8, 2, and 0 in that order\line
If an argument can only accept one type of feature, then the feature can be specified implicitly:\line
{\f1\fs14 1}\tab\tab The vertex with index 1\line
{\f1\fs14 4 9}\tab\tab The edge connecting vertex 4 and vertex 9\line
{\f1\fs14 8 2 0}\tab\tab The face with vertices 8, 2, and 0 in that order\line
Some arguments accept vectors which can usually be specified implicitly, but in cases where an argument can
 be either a vector or a feature, the vector must be specified explicitly:\line
{\f1\fs14 1.0 2 -3.0}\tab Implicit vector with x=1.0, y=2.0, and z=-3.0\line
{\f1\fs14 (1.0 2 -3.0)}\tab Explicit vector with x=1.0, y=2.0, and z=-3.0\line
Some commands that accept a face as an argument will try to act on whatever face is a "superset" and/or
 an inversion of the given argument. A face is a "superset" of another if it has all of the other face's
 vertices, and if the vertices appear throughout the face in the same order. Commands that try to match faces
 in this way assume that no two faces can share more than three vertices. This face matching functionality
 makes commands like {\f1 :df} (delete face) easier to use: instead of specifying all vertices of a face you
 wish to delete, you can just specify three of them, and in any order.
\par})";

int help_command_impl::handle(fcad_start_event &event) {
	noop_command_impl::handle(event);
	std::ostringstream oss{};

	oss << help_text_intro;

	write_help_rtf_header(oss, "Controls");
	write_help_rtf_row(oss, "HJKL",
		"Rotates the camera about the target. The target is at the origin by default."
	);
	write_help_rtf_row(oss, "B<PERIOD>",
		"Rolls the camera about the radial axis (from the camera position to the target)."
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
	oss << commands_intro;
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