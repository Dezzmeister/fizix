#include "commands.h"
#include "helpers.h"
#include "parsing.h"

void delete_face_command_impl::on_submit(const std::wstring &args) {
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

	if (geom->delete_face(*face_opt)) {
		history->add_command(L":df " + args);
	}
}

void delete_face_command_impl::write_help_text(std::ostream &os) const {
	write_help_rtf_row(os, ":df <v1> <v2> ... <vN>",
		"Deletes a face given at least three vertex indices. "
		"The vertices can be specified in clockwise or counter-clockwise order."
	);
}