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

	if (! state.eof()) {
		platform->set_cue_text(L"Unexpected trailing chars");
		return;
	}

	if (! feat_idx_opt) {
		platform->set_cue_text(L"Expected an explicit feature");
		return;
	}

	std::optional<feature> feat_opt = geom->get_feature(*feat_idx_opt);

	if (! feat_opt) {
		return;
	}

	polyhedron p = geom->get_poly().group(*feat_opt);

	clipboard->add_poly(clipboard_controller::selection_name(), p);
	history->add_command(L":yg " + args);
	return;
}

void yank_group_command_impl::write_help_text(std::ostream &os) const {
	write_help_rtf_row(os, ":yg <TODO>", "TODO");
}