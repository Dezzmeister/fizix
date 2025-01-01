#include "commands.h"
#include "helpers.h"
#include "parsing.h"

void flip_command_impl::on_submit(const std::wstring &args) {
	std::wstringstream wss{};
	wss << args;

	parsing::parser_state state(wss);
	parsing::parse_whitespace(state);
	std::optional<face> face_opt = parse_implicit_face(state);

	if (! face_opt) {
		return;
	}

	parsing::parse_whitespace(state);

	if (! state.eof()) {
		return;
	}

	geom->flip(*face_opt);
	history->add_command(L":flip " + args);
}

void flip_command_impl::write_help_text(std::ostream &os) const {
	write_help_rtf_row(os, ":flip <v1> <v2> ... <vN>",
		"Flips a face. The face's vertex indices can be given in either "
		"clockwise or counter-clockwise winding order."
	);
}