#include <logging.h>
#include <util.h>
#include "commands.h"
#include "parsing.h"

using namespace phys;

create_vertex_command_impl::create_vertex_command_impl(fcad_event_bus &_events) :
	events(_events) {}

void create_vertex_command_impl::on_submit(const std::wstring &args) {
	std::wstringstream wss{};
	wss << args;
	parsing::parser_state state(wss);
	parsing::parse_whitespace(state);

	partial_vec3 pos_opt = parse_vec3(state);
	std::optional<vec3> vertex_opt = pos_opt.try_as_vec3();

	if (! vertex_opt) {
		return;
	}

	new_vertex_event event(*vertex_opt);
	events.fire(event);
}