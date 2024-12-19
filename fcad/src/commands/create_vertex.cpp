#include <logging.h>
#include <util.h>
#include "commands.h"
#include "parsing.h"

using namespace phys;

struct vertex_coords {
	std::optional<real> x{};
	std::optional<real> y{};
	std::optional<real> z{};

	vertex_coords(const std::wstring &args) {
		std::wstringstream wss{};
		std::wstringstream sink{};

		wss << args;

		parsing::parser_state state(wss);
		x = parse_real(state);

		parsing::parse_one_char(state, L',', sink);
		parsing::parse_whitespace(state);

		y = parse_real(state);

		parsing::parse_one_char(state, L',', sink);
		parsing::parse_whitespace(state);

		z = parse_real(state);
	}
};

create_vertex_command_impl::create_vertex_command_impl(fcad_event_bus &_events) :
	events(_events) {}

void create_vertex_command_impl::on_cancel(const std::wstring&) {}
void create_vertex_command_impl::on_input(const std::wstring&) {}
void create_vertex_command_impl::on_submit(const std::wstring &args) {
	vertex_coords pos_opt(args);

	if (! pos_opt.z || ! pos_opt.y || ! pos_opt.x) {
		return;
	}

	vec3 vertex(*pos_opt.x, *pos_opt.y, *pos_opt.z);
	logger::debug(vertex);

	new_vertex_event event(vertex);
	events.fire(event);
}