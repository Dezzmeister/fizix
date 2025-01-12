#include "commands.h"
#include "helpers.h"
#include "parsing.h"

void delete_group_command_impl::on_submit(const std::wstring &args) {
	std::wstringstream wss{};
	wss << args;
	parsing::parser_state state(wss);
	parsing::parse_whitespace(state);

	std::optional<index_feature> group_feat_idx_opt = parse_explicit_feature(state);

	parsing::parse_whitespace(state);

	if (! group_feat_idx_opt) {
		platform->set_cue_text(L"Expected an explicit feature");
		return;
	}

	if (! state.eof()) {
		platform->set_cue_text(L"Unexpected trailing chars");
		return;
	}

	std::optional<feature> group_feat_opt = geom->get_feature(*group_feat_idx_opt);

	if (! group_feat_opt) {
		return;
	}

	polyhedron group = geom->get_poly().group(*group_feat_opt);
	geom->delete_features(group);
	history->add_command(L":dg " + args);
}

void delete_group_command_impl::write_help_text(std::ostream &os) const {
	write_help_rtf_row(os, ":dg <feature>",
		"TODO"
	);
}