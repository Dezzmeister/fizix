#pragma once
#include "action.h"

class start_command_impl : public action_impl {
public:
	start_command_impl(fcad_event_bus &_events);

	void on_accept(char c) override;

private:
	fcad_event_bus &events;
};