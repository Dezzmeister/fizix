#include "commands.h"
#include "helpers.h"
#include "parsing.h"

void yank_face_command_impl::on_submit(const std::wstring &args) {
	std::wstringstream wss{};
	wss << args;
	parsing::parser_state state(wss);
	parsing::parse_whitespace(state);

	std::optional<face> face_opt = parse_implicit_face(state);

	if (! face_opt) {
		platform->set_cue_text(L"Expected an implicit face");
		return;
	}

	std::optional<face> match_opt = geom->get_matching_face(*face_opt);

	if (! match_opt) {
		return;
	}

	polyhedron p = geom->get_poly().isolated(*match_opt);

	parsing::parse_whitespace(state);

	if (state.eof()) {
		std::optional<vec3> centroid_opt = geom->centroid(*match_opt);

		if (! centroid_opt) {
			return;
		}

		vec3 centroid = *centroid_opt;

		for (vertex &v : p.vertices) {
			v.v -= centroid;
		}

		clipboard->add_poly(clipboard_controller::selection_name(), p);
		history->add_command(L":yf " + args);
		return;
	}

	std::optional<vec3_or_index_feature> vec3_or_index_opt = parse_explicit_vec3_or_feature(state);

	if (! vec3_or_index_opt) {
		platform->set_cue_text(L"Expected an explicit vector or feature");
		return;
	}

	parsing::parse_whitespace(state);

	if (! state.eof()) {
		platform->set_cue_text(L"Unexpected trailing chairs");
		return;
	}

	std::optional<vec3> pos_opt = geom->centroid(*vec3_or_index_opt);

	if (! pos_opt) {
		return;
	}

	vec3 pos = *pos_opt;

	for (vertex &v : p.vertices) {
		v.v -= pos;
	}

	clipboard->add_poly(clipboard_controller::selection_name(), p);
	history->add_command(L":yf " + args);
}

void yank_face_command_impl::write_help_text(std::ostream &os) const {
	write_help_rtf_row(os, ":yf <v1> <v2> ... <vN> [vector\\~or\\~feature]",
		"Copies a face. The face to be copied is specified implicitly. If "
		"an optional explicit vector is provided, the face will be copied "
		"with reference to that vector. If an explicit feature is provided "
		"instead, the face will be copied with reference to the feature's "
		"centroid."
	);
}