#include "commands.h"
#include "helpers.h"
#include "parsing.h"

void focus_command_impl::on_submit(const std::wstring &args) {
	std::wstringstream wss{};
	wss << args;
	parsing::parser_state state(wss);
	parsing::parse_whitespace(state);

	if (state.eof()) {
		camera->set_target(vec3{});
		return;
	}

	std::optional<vec3> target_opt = parse_explicit_vec3(state);

	if (target_opt) {
		parsing::parse_whitespace(state);

		if (state.eof()) {
			camera->set_target(*target_opt);
			return;
		} else {
			platform->set_cue_text(L"Unexpected char(s) after vector");
			return;
		}
	}

	std::optional<index_feature> feat_opt = parse_explicit_feature(state);

	if (! feat_opt) {
		return;
	}

	const index_feature &feat = *feat_opt;

	if (std::holds_alternative<size_t>(feat)) {
		target_opt = geom->vertex_pos(std::get<size_t>(feat));
	} else if (std::holds_alternative<edge>(feat)) {
		target_opt = geom->centroid(std::get<edge>(feat));
	} else if (std::holds_alternative<face>(feat)) {
		face f = std::get<face>(feat);
		std::optional<face> face_to_focus_opt = geom->superset_face(f);

		if (! face_to_focus_opt) {
			face_to_focus_opt = geom->superset_face(f.flipped());
		}

		if (face_to_focus_opt) {
			f = *face_to_focus_opt;
		}

		target_opt = geom->centroid(f);
	}

	if (! target_opt) {
		return;
	}

	parsing::parse_whitespace(state);

	if (! state.eof()) {
		platform->set_cue_text(L"Unexpected char(s) after explicit feature");
		return;
	}

	camera->set_target(*target_opt);
}

void focus_command_impl::write_help_text(std::ostream &os) const {
	write_help_rtf_row(os, ":focus [(<x>\\~<y>\\~<z>) | v<i> | e(<v1>\\~<v2>) | f(<v1>\\~<v2>\\~...\\~<vN>)]",
		"Moves the target. The argument is optional: if not provided, the target "
		// The non-breaking spaces here don't seem to work. Is this a Rich Edit bug?
		"will be set to (0,\\~0,\\~0). The argument can either be an explicit vector, "
		"explicit vertex, explicit edge, or explicit face. If the argument is a vector, "
		"the target will be set to that position. If the argument is a feature, the target "
		"will be set to the centroid of the feature."
	);
}