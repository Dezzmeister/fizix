#pragma once
#include <events.h>
#include "fcad_events.h"

class mode_controller :
	traits::pinned<mode_controller>,
	public event_listener<program_start_event>
{
public:
	mode_controller(
		event_buses &_buses,
		fcad_event_bus &_events
	);

	void set_mode(edit_mode new_mode, const std::wstring &command_hint = L"");

	int handle(program_start_event &event) override;

private:
	fcad_event_bus &events;
	edit_mode curr_mode{ edit_mode::Normal };
};