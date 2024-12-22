#include <shader_constants.h>
#include <util.h>
#include "camera_controller.h"

camera_controller::camera_controller(
	event_buses &_buses,
	fcad_event_bus &_events
) :
	event_listener<shader_use_event>(&_buses.render),
	event_listener<draw_event>(&_buses.render),
	event_listener<program_start_event>(&_buses.lifecycle),
	event_listener<screen_resize_event>(&_buses.render),
	event_listener<camera_move_event>(&_events)
{
	event_listener<shader_use_event>::subscribe();
	event_listener<draw_event>::subscribe();
	event_listener<program_start_event>::subscribe();
	event_listener<screen_resize_event>::subscribe();
	event_listener<camera_move_event>::subscribe();

	update_view_mat();
}

int camera_controller::handle(shader_use_event &event) {
	static constexpr int view_loc = util::find_in_map(constants::shader_locs, "view");
	static constexpr int inv_view_loc = util::find_in_map(constants::shader_locs, "inv_view");
	static constexpr int projection_loc = util::find_in_map(constants::shader_locs, "projection");

	event.shader.set_uniform(view_loc, view);
	event.shader.set_uniform(inv_view_loc, inv_view);
	event.shader.set_uniform(projection_loc, projection);

	return 0;
}

int camera_controller::handle(draw_event &event) {
	event.view = &view;
	event.inv_view = &inv_view;

	return 0;
}

int camera_controller::handle(program_start_event &event) {
	update_projection_mat(event.screen_width, event.screen_height);

	return 0;
}

int camera_controller::handle(screen_resize_event &event) {
	update_projection_mat(event.new_width, event.new_height);

	return 0;
}

int camera_controller::handle(camera_move_event &event) {
	// TODO
	logger::debug(event.dir);

	return 0;
}

void camera_controller::update_projection_mat(int width, int height) {
	projection = glm::perspective(
		glm::radians(60.0f),
		width / (float)height,
		0.1f,
		100.0f
	);
}

void camera_controller::update_view_mat() {
	view = glm::lookAt(pos, glm::vec3(0.0f), up);
	inv_view = glm::inverse(view);
}