#include "commands.h"
#include "parsing.h"

delete_edge_command_impl::delete_edge_command_impl(fcad_event_bus &_events) :
	events(_events) {}

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
	delete_edge_event event(e);
	events.fire(event);
}