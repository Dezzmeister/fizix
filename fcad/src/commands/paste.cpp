#include "commands.h"
#include "helpers.h"
#include "parsing.h"

void paste_command_impl::on_submit(const std::wstring &args) {
	std::wstringstream wss{};
	wss << args;
	parsing::parser_state state(wss);
	std::optional<vec3_or_index_feature> vec3_or_index_opt = parse_explicit_vec3_or_feature(state);

	if (! vec3_or_index_opt) {
		platform->set_cue_text(L"Expected a vector or vertex");
		return;
	}

	vec3_or_index_feature vec3_or_index = *vec3_or_index_opt;
	vec3 pos{};

	if (std::holds_alternative<vec3>(vec3_or_index)) {
		pos = std::get<vec3>(vec3_or_index);
	} else if (std::holds_alternative<size_t>(vec3_or_index)) {
		size_t vertex_idx = std::get<size_t>(vec3_or_index);
		std::optional<vec3> pos_opt = geom->centroid(vertex_idx);

		if (! pos_opt) {
			return;
		}

		pos = *pos_opt;
	} else {
		platform->set_cue_text(L"Expected a vector or vertex");
		return;
	}

	const polyhedron * p = clipboard->get_poly(clipboard_controller::selection_name());

	if (! p) {
		platform->set_cue_text(L"Current selection is empty");
		return;
	}

	geom->add_poly_at(*p, pos);
	history->add_command(L":p " + args);
}

void paste_command_impl::write_help_text(std::ostream &os) const {
	write_help_rtf_row(os, ":p (<x>\\~<y>\\~<z>)\\~|\\~<v1>",
		"Pastes the selection at the given position or vertex."
	);
}