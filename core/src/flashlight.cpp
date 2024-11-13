#include "flashlight.h"

flashlight::flashlight(event_buses &_buses, const player &_pl, world &_w, const short _toggle_key) :
	event_listener<pre_render_pass_event>(&_buses.render, -25),
	event_listener<keydown_event>(&_buses.input),
	light(std::make_unique<spotlight>(
		glm::vec3(0.0f),
		glm::vec3(0.0f),
		glm::radians(12.5f),
		glm::radians(17.5f),
		light_properties(
			glm::vec3(0.1f),
			glm::vec3(0.25f),
			glm::vec3(1.0f)
		),
		attenuation_factors(
			1.0f,
			0.027f,
			0.0028f
		)
	)),
	pl(_pl),
	w(_w),
	toggle_key(_toggle_key),
	enabled(false),
	added_to_world(false)
{
	event_listener<pre_render_pass_event>::subscribe();
	event_listener<keydown_event>::subscribe();
}

int flashlight::handle(pre_render_pass_event&) {
	if (enabled) {
		const camera &cam = pl.get_camera();
		light->pos = cam.pos;
		light->dir = cam.dir;

		if (! added_to_world) {
			w.add_light(light.get());
			added_to_world = true;
		}
	}

	return 0;
}

int flashlight::handle(keydown_event &event) {
	if (event.key == toggle_key) {
		enabled = !enabled;

		if (enabled) {
			added_to_world = false;
		} else {
			w.remove_light(light.get());
		}
	}

	return 0;
}
