#include <logging.h>
#include <util.h>
#include "commands.h"
#include "helpers.h"
#include "parsing.h"

using namespace phys;

struct edge_indices {
	std::optional<size_t> start{};
	std::optional<size_t> end{};

	edge_indices(const std::wstring &args) {
		std::wstringstream wss{};
		std::wstringstream sink{};

		wss << args;

		parsing::parser_state state(wss);
		parsing::parse_whitespace(state);
		start = parse_size(state);

		parsing::parse_one_char(state, L',', sink);
		parsing::parse_whitespace(state);

		end = parse_size(state);
	}
};

void create_edge_command_impl::on_submit(const std::wstring &args) {
	edge_indices edge_opt(args);

	if (! edge_opt.start || ! edge_opt.end) {
		return;
	}

	edge e(*edge_opt.start, *edge_opt.end);

	if (geom->create_edge(e)) {
		history->add_command(L":e " + args);
	}
}

void create_edge_command_impl::write_help_text(std::ostream &os) const {
	write_help_rtf_row(os, ":e <v1> <v2>", "Creates an edge given two vertex indices.");
}
