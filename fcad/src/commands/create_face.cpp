#include <logging.h>
#include <util.h>
#include "commands.h"
#include "helpers.h"
#include "parsing.h"

void create_face_command_impl::on_submit(const std::wstring &args) {
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

	new_face_event event(*face_opt);

	if (! events.fire(event)) {
		history->add_command(L":f " + args);
	}
}

void create_face_command_impl::write_help_text(std::ostream &os) const {
	write_help_rtf_row(os, ":f <v1> <v2> ... <vN>",
		"Creates a face given at least 3 vertex indices. "
		"The winding order of the vertices defines the face's direction. "
		"The \"outer\" side (the direction towards which the normal points) "
		"is the one where the vertices are in counter-clockwise winding order. "
		"The \"outer\" side will be colored gray, and the \"inner\" side will be purple. "
		"The vertices must be coplanar, and the face must not self-intersect. "
		"The face may be convex or nonconvex."
	);
}
