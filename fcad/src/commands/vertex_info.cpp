#include "commands.h"
#include "helpers.h"
#include "parsing.h"

using namespace phys;

void vertex_info_command_impl::on_submit(const std::wstring &args) {
	using namespace traits;

	std::wstringstream wss{};
	wss << args;
	parsing::parser_state state(wss);
	parsing::parse_whitespace(state);

	std::optional<size_t> vert_idx_opt = parse_size(state);

	if (! vert_idx_opt) {
		return;
	}

	parsing::parse_whitespace(state);

	if (! state.eof()) {
		return;
	}

	std::optional<vec3> pos = geom->get_vertex_pos(*vert_idx_opt);

	if (! pos) {
		return;
	}

	platform->set_cue_text("Vertex " + to_string(*vert_idx_opt) + ": " + fmt_vec3(*pos));
}

void vertex_info_command_impl::write_help_text(std::ostream &os) const {
	write_help_rtf_row(os, ":n <v1>",
		"Prints the position of a vertex given its index."
	);
}