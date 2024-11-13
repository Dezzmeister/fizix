#include "gl.h"
#include "hardware_constants.h"

hardware_constants::hardware_constants(event_buses &_buses) :
	event_listener<program_start_event>(&_buses.lifecycle, -100)
{
	event_listener<program_start_event>::subscribe();
}

int hardware_constants::handle(program_start_event &event) {
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_texture_units);

	initialized = true;
	event.hardware_consts = this;

	return 0;
}

int hardware_constants::get_max_texture_units() const {
	guard();

	return max_texture_units;
}

void hardware_constants::guard() const {
	if (! initialized) {
		// TODO: Errors
		throw "Hardware constants are not initialized";
	}
}

