#pragma once
#include "action.h"
#include "fcad_events.h"

class window_action_controller :
	public event_listener<window_input_event>
{
public:
	window_action_controller(
		fcad_event_bus &_events,
		action_group &_window_actions
	);

	int handle(window_input_event &event) override;

private:
	action_group &window_actions;
};