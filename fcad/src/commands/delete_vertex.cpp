#include "commands.h"
#include "parsing.h"

using namespace phys;

void delete_vertex_command_impl::on_submit(const std::wstring &args) {
	std::wstringstream wss{};
	wss << args;
	parsing::parser_state state(wss);
	parsing::parse_whitespace(state);

	std::optional<size_t> vert_idx_opt = parse_size(state);

	if (! vert_idx_opt) {
		return;
	}

	parsing::parse_whitespace(state);

	if (! state.eof()) {
		return;
	}

	delete_vertex_event event(*vert_idx_opt);

	if (! events.fire(event)) {
		history->add_command(L":dv " + args);
	}
}