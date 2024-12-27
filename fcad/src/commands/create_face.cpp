#include <logging.h>
#include <util.h>
#include "commands.h"
#include "parsing.h"

create_face_command_impl::create_face_command_impl(fcad_event_bus &_events) :
	events(_events) {}

void create_face_command_impl::on_submit(const std::wstring &args) {
	std::wstringstream wss{};
	wss << args;

	parsing::parser_state state(wss);
	parsing::parse_whitespace(state);
	std::optional<face> face_opt = parse_implicit_face(state);

	if (! face_opt) {
		return;
	}

	parsing::parse_whitespace(state);

	if (! state.eof()) {
		return;
	}

	new_face_event event(*face_opt);
	events.fire(event);
}
