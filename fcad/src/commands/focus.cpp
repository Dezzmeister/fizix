#include "commands.h"
#include "parsing.h"

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

	camera->set_target(new_target);
}