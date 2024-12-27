#include "command_controller.h"
#include "commands.h"

command_controller::command_controller(
	fcad_event_bus &_events,
	std::unordered_map<std::wstring, std::unique_ptr<command_impl>> &&_command_impls
) :
	event_listener<command_cancel_event>(&_events),
	event_listener<command_input_event>(&_events),
	event_listener<command_submit_event>(&_events),
	command_impls(std::move(_command_impls))
{
	event_listener<command_cancel_event>::subscribe();
	event_listener<command_input_event>::subscribe();
	event_listener<command_submit_event>::subscribe();
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
		// TODO: Show error message to user
		logger::debug(
			"User submitted nonexistent command: " +
			traits::to_string(command)
		);

		return 0;
	}

	pair->second->on_submit(rest);

	return 0;
}

command_controller make_commands(event_buses&, fcad_event_bus &events) {
	std::unordered_map<std::wstring, std::unique_ptr<command_impl>> impls{};

	impls.emplace(std::make_pair(L"q", std::make_unique<quit_command_impl>()));
	impls.emplace(std::make_pair(L"focus", std::make_unique<focus_command_impl>(events)));
	impls.emplace(std::make_pair(L"v", std::make_unique<create_vertex_command_impl>(events)));
	impls.emplace(std::make_pair(L"e", std::make_unique<create_edge_command_impl>(events)));
	impls.emplace(std::make_pair(L"f", std::make_unique<create_face_command_impl>(events)));
	impls.emplace(std::make_pair(L"dv", std::make_unique<delete_vertex_command_impl>(events)));

	return command_controller(events, std::move(impls));
}