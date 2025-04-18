#include "commands.h"
#include "helpers.h"
#include "parsing.h"

void delete_edge_command_impl::on_submit(const std::wstring &args) {
	std::wstringstream wss{};
	wss << args;
	parsing::parser_state state(wss);

	parsing::parse_whitespace(state);
	std::optional<size_t> v1_opt = parse_size(state);

	if (! v1_opt) {
		return;
	}

	parsing::parse_whitespace(state);
	std::optional<size_t> v2_opt = parse_size(state);

	if (! v2_opt) {
		return;
	}

	parsing::parse_whitespace(state);

	if (! state.eof()) {
		return;
	}

	edge e(*v1_opt, *v2_opt);

	if (geom->delete_edge(e)) {
		history->add_command(L":de " + args);
	}
}

void delete_edge_command_impl::write_help_text(std::ostream &os) const {
	write_help_rtf_row(os, ":de <v1> <v2>",
		"Deletes an edge given two vertex indices."
	);
}