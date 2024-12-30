#pragma once
#include <map>
#include <events.h>
#include "../command.h"
#include "fcad_events.h"

class command_controller :
	public event_listener<command_cancel_event>,
	public event_listener<command_input_event>,
	public event_listener<command_submit_event>
{
public:
	command_controller(
		fcad_event_bus &_events,
		std::map<std::wstring, std::unique_ptr<command_impl>> &&_command_impls
	);

	void write_help_text(std::ostream &os) const;

	int handle(command_cancel_event &event) override;
	int handle(command_input_event &event) override;
	int handle(command_submit_event &event) override;

private:
	std::map<std::wstring, std::unique_ptr<command_impl>> command_impls;
};

command_controller make_commands(event_buses&, fcad_event_bus &events);