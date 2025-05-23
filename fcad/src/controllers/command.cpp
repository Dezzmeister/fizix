#include "commands.h"
#include "controllers/command.h"
#include "fcad_platform/platform.h"

command_controller::command_controller(
	fcad_event_bus &_fcad_events,
	std::map<std::wstring, std::unique_ptr<command_impl>> &&_command_impls
) :
	event_listener<fcad_start_event>(&_fcad_events),
	event_listener<command_cancel_event>(&_fcad_events),
	event_listener<command_input_event>(&_fcad_events),
	event_listener<command_submit_event>(&_fcad_events),
	command_impls(std::move(_command_impls))
{
	event_listener<fcad_start_event>::subscribe();
	event_listener<command_cancel_event>::subscribe();
	event_listener<command_input_event>::subscribe();
	event_listener<command_submit_event>::subscribe();
}

void command_controller::write_help_text(std::ostream &os) const {
	for (const auto &pair : command_impls) {
		pair.second->write_help_text(os);
	}
}

int command_controller::handle(fcad_start_event &event) {
	platform = &event.platform;

	return 0;
}

int command_controller::handle(command_cancel_event &event) {
	if (! event.command_buf.starts_with(L":")) {
		return 0;
	}

	std::wstring::size_type space_loc = event.command_buf.find(L' ');
	std::wstring command = event.command_buf.substr(1, space_loc - 1);
	std::wstring rest = L"";

	if (space_loc != std::wstring::npos) {
		rest = event.command_buf.substr(space_loc + 1);
	}

	auto pair = command_impls.find(command);

	if (pair == std::end(command_impls)) {
		logger::debug(
			"User cancelled nonexistent command: " +
			traits::to_string(command)
		);

		return 0;
	}

	pair->second->on_cancel(rest);

	return 0;
}

int command_controller::handle(command_input_event &event) {
	if (! event.command_buf.starts_with(L":")) {
		return 0;
	}

	std::wstring::size_type space_loc = event.command_buf.find(L' ');
	std::wstring command = event.command_buf.substr(1, space_loc - 1);
	std::wstring rest = L"";

	if (space_loc != std::wstring::npos) {
		rest = event.command_buf.substr(space_loc + 1);
	}

	auto pair = command_impls.find(command);

	if (pair == std::end(command_impls)) {
		return 0;
	}

	pair->second->on_input(rest);

	return 0;
}

int command_controller::handle(command_submit_event &event) {
	if (! event.command.starts_with(L":")) {
		return 0;
	}

	std::wstring::size_type space_loc = event.command.find(L' ');
	std::wstring command = event.command.substr(1, space_loc - 1);
	std::wstring rest = L"";

	if (space_loc != std::wstring::npos) {
		rest = event.command.substr(space_loc + 1);
	}

	auto pair = command_impls.find(command);

	if (pair == std::end(command_impls)) {
		logger::debug(
			"User submitted nonexistent command: " +
			traits::to_string(command)
		);
		platform->set_cue_text(L"\"" + command + L"\" does not exist");

		return 0;
	}

	pair->second->on_submit(rest);

	return 0;
}

command_controller make_commands(event_buses&, fcad_event_bus &events) {
	std::map<std::wstring, std::unique_ptr<command_impl>> impls{};

	impls.emplace(std::make_pair(L"q", std::make_unique<quit_command_impl>(events)));
	impls.emplace(std::make_pair(L"focus", std::make_unique<focus_command_impl>(events)));
	impls.emplace(std::make_pair(L"v", std::make_unique<create_vertex_command_impl>(events)));
	impls.emplace(std::make_pair(L"e", std::make_unique<create_edge_command_impl>(events)));
	impls.emplace(std::make_pair(L"f", std::make_unique<create_face_command_impl>(events)));
	impls.emplace(std::make_pair(L"dv", std::make_unique<delete_vertex_command_impl>(events)));
	impls.emplace(std::make_pair(L"de", std::make_unique<delete_edge_command_impl>(events)));
	impls.emplace(std::make_pair(L"df", std::make_unique<delete_face_command_impl>(events)));
	impls.emplace(std::make_pair(L"w", std::make_unique<write_file_command_impl>(events)));
	impls.emplace(std::make_pair(L"r", std::make_unique<read_file_command_impl>(events)));
	impls.emplace(std::make_pair(L"labeltype", std::make_unique<labeltype_command_impl>(events)));
	impls.emplace(std::make_pair(L"h", std::make_unique<help_command_impl>(events)));
	impls.emplace(std::make_pair(L"flip", std::make_unique<flip_command_impl>(events)));
	impls.emplace(std::make_pair(L"yf", std::make_unique<yank_face_command_impl>(events)));
	impls.emplace(std::make_pair(L"yg", std::make_unique<yank_group_command_impl>(events)));
	impls.emplace(std::make_pair(L"p", std::make_unique<paste_command_impl>(events)));
	impls.emplace(std::make_pair(L"mg", std::make_unique<move_group_command_impl>(events)));
	impls.emplace(std::make_pair(L"dg", std::make_unique<delete_group_command_impl>(events)));
	impls.emplace(std::make_pair(L"n", std::make_unique<vertex_info_command_impl>(events)));
	impls.emplace(std::make_pair(L"let", std::make_unique<let_command_impl>(events)));
	impls.emplace(std::make_pair(L"set", std::make_unique<set_command_impl>(events)));
	impls.emplace(std::make_pair(L"pref", std::make_unique<pref_command_impl>(events)));

	return command_controller(events, std::move(impls));
}