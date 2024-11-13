#pragma once
#include "events.h"

class hardware_constants : public event_listener<program_start_event> {
public:

	hardware_constants(event_buses &_buses);

	int handle(program_start_event &event) override;

	int get_max_texture_units() const;

private:
	bool initialized{ false };
	int max_texture_units{ -1 };

	void guard() const;
};
