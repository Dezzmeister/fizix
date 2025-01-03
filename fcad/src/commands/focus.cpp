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

	std::optional<vec3_or_index_feature> vec3_or_index_opt = parse_explicit_vec3_or_feature(state);

	if (! vec3_or_index_opt) {
		platform->set_cue_text(L"Expected an explicit vector or feature");
		return;
	}

	parsing::parse_whitespace(state);

	if (! state.eof()) {
		platform->set_cue_text(L"Unexpected trailing chars");
		return;
	}

	std::optional<vec3> pos_opt = geom->centroid(*vec3_or_index_opt);

	if (! pos_opt) {
		return;
	}

	camera->set_target(*pos_opt);
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