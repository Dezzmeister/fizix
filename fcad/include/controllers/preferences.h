#pragma once
#include <fcad_events.h>

class preferences_controller :
	traits::pinned<preferences_controller>,
	public event_listener<fcad_start_event>
{
public:
	preferences_controller(fcad_event_bus &_events);

	int handle(fcad_start_event &event) override;
};