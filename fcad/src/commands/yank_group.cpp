#include "commands.h"
#include "helpers.h"
#include "parsing.h"

void yank_group_command_impl::on_submit(const std::wstring &args) {
	std::wstringstream wss{};
	wss << args;
	parsing::parser_state state(wss);
	parsing::parse_whitespace(state);

	std::optional<index_feature> feat_idx_opt = parse_explicit_feature(state);

	parsing::parse_whitespace(state);

	if (! feat_idx_opt) {
		platform->set_cue_text(L"Expected an explicit feature");
		return;
	}

	std::optional<feature> feat_opt = geom->get_feature(*feat_idx_opt);

	if (! feat_opt) {
		return;
	}

	if (state.eof()) {
		polyhedron p = geom->get_poly().group(*feat_opt);
		vec3 centroid = geom->get_poly().centroid();

		clipboard->add_poly(clipboard_controller::selection_name(), p.translated(-centroid));
		history->add_command(L":yg " + args);
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

	polyhedron p = geom->get_poly().group(*feat_opt);
	vec3 pos = *pos_opt;

	clipboard->add_poly(clipboard_controller::selection_name(), p.translated(-pos));
	history->add_command(L":yg " + args);
}

void yank_group_command_impl::write_help_text(std::ostream &os) const {
	write_help_rtf_row(os, ":yg <feature> [vector or feature]",
		"Copies a collection of features. The features connected to the "
		"first argument will be copied with reference to the second argument, "
		"or to the centroid of the selection if no second argument is provided."
	);
}