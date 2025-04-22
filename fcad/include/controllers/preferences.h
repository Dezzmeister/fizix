#pragma once
#include <fcad_events.h>

class preferences_controller :
	traits::pinned<preferences_controller>,
	public event_listener<fcad_start_event>
{
public:
	preferences_controller(fcad_event_bus &_events);

	void set_log_cmd_output(bool _log);
	bool log_cmd_output() const;
	int handle(fcad_start_event &event) override;

private:
	bool _log_cmd_output{};
};