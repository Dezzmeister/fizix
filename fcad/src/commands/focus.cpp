#include "commands.h"
#include "parsing.h"

focus_command_impl::focus_command_impl(fcad_event_bus &_events) :
	events(_events) {}

void focus_command_impl::on_submit(const std::wstring &args) {
	std::wstringstream wss{};
	wss << args;
	parsing::parser_state state(wss);
	parsing::parse_whitespace(state);
	std::optional<vec3> target_opt = parse_vec3(state).try_as_vec3();
	vec3 new_target{};

	if (target_opt) {
		new_target = *target_opt;
	}

	set_camera_target_event event(new_target);
	events.fire(event);
}