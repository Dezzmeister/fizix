#include "commands.h"
#include "helpers.h"
#include "parsing.h"

void move_group_command_impl::on_submit(const std::wstring &args) {
	std::wstringstream wss{};
	wss << args;
	parsing::parser_state state(wss);
	parsing::parse_whitespace(state);

	std::optional<index_feature> group_feat_idx_opt = parse_explicit_feature(state);

	parsing::parse_whitespace(state);

	if (! group_feat_idx_opt) {
		set_output(L"Expected an explicit feature");
		return;
	}

	std::optional<feature> group_feat_opt = geom->get_feature(*group_feat_idx_opt);

	if (! group_feat_opt) {
		return;
	}

	std::optional<vec3_or_index_feature> ref_opt = parse_explicit_vec3_or_feature(state);

	parsing::parse_whitespace(state);

	if (! ref_opt) {
		set_output(L"Expected an explicit feature");
		return;
	}

	std::optional<vec3> ref_pos_opt = geom->centroid(*ref_opt);

	if (! ref_pos_opt) {
		return;
	}

	std::optional<vec3_or_index_feature> to_opt = parse_explicit_vec3_or_feature(state);

	if (! to_opt) {
		set_output(L"Expected an explicit feature or vector");
		return;
	}

	parsing::parse_whitespace(state);

	if (! state.eof()) {
		set_output(L"Unexpected trailing chars");
		return;
	}

	std::optional<vec3> centroid_opt = geom->centroid(*to_opt);

	if (! centroid_opt) {
		return;
	}

	const feature &group_feat = *group_feat_opt;
	const vec3 &ref_pos = *ref_pos_opt;
	const vec3 &to_pos = *centroid_opt;

	polyhedron group = geom->get_poly().group(group_feat);
	geom->move_features(group, to_pos - ref_pos);
	history->add_command(L":mg " + args);
}

void move_group_command_impl::write_help_text(std::ostream &os) const {
	write_help_rtf_row(os, ":mg <feature> <origin> <to>",
		"Moves a group of features. The features to be moved are those "
		"connected to {\\b\\f1 <feature>}. The group is moved from "
		"{\\b\\f1 <origin>} to {\\b\\f1 <to>}."
	);
}