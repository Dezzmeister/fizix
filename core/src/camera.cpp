#include <stdio.h>
#include "camera.h"
#include "shader_constants.h"
#include "util.h"

camera::camera(event_buses &_buses) :
	event_listener<pre_render_pass_event>(&_buses.render),
	event_listener<shader_use_event>(&_buses.render),
	event_listener<draw_event>(&_buses.render, -100),
	event_listener<screen_resize_event>(&_buses.render),
	pos(glm::vec3(0.0f, 0.0f, 0.0f)),
	dir(glm::vec3(0.0f, 0.0f, -1.0f)),
	world_up(glm::vec3(0.0f, 1.0f, 0.0f)),
	right(glm::normalize(glm::cross(world_up, dir))),
	camera_up(glm::cross(dir, right)),
	target(glm::vec3(0.0f, 0.0f, 1.0f)),
	view(glm::mat4(1.0f)),
	inv_view(glm::inverse(view)),
	projection(glm::perspective(glm::radians(60.0f), 800.0f / 600.0f, 0.1f, 100.0f))
{
	event_listener<pre_render_pass_event>::subscribe();
	event_listener<shader_use_event>::subscribe();
	event_listener<draw_event>::subscribe();
	event_listener<screen_resize_event>::subscribe();
}

int camera::handle(pre_render_pass_event&) {
	update_view_mat();

	return 0;
}

// TODO: Don't set uniforms that don't exist
int camera::handle(shader_use_event &event) {
	constexpr int view_loc = util::find_in_map(constants::shader_locs, "view");
	constexpr int inv_view_loc = util::find_in_map(constants::shader_locs, "inv_view");
	constexpr int projection_loc = util::find_in_map(constants::shader_locs, "projection");

	event.shader.set_uniform(view_loc, view);
	event.shader.set_uniform(inv_view_loc, inv_view);
	event.shader.set_uniform(projection_loc, projection);

	return 0;
}

int camera::handle(draw_event &event) {
	event.view = &view;
	event.inv_view = &inv_view;

	return 0;
}

int camera::handle(screen_resize_event &event) {
	projection = glm::perspective(
		glm::radians(60.0f),
		event.new_width / ((float)event.new_height),
		0.1f,
		100.0f
	);

	return 0;
}

void camera::update_right_vec() {
	right = glm::normalize(glm::cross(world_up, dir));
}

void camera::update_view_mat() {
	right = glm::normalize(glm::cross(world_up, dir));
	camera_up = glm::cross(dir, right);
	target = pos + dir;
	view = glm::lookAt(pos, target, world_up);
	inv_view = glm::inverse(view);
}

const glm::mat4& camera::get_view() const {
	return view;
}
