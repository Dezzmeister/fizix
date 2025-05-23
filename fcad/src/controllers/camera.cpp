#include <shader_constants.h>
#include <util.h>
#include "controllers/camera.h"

camera_controller::camera_controller(
	event_buses &_buses,
	fcad_event_bus &_events
) :
	event_listener<pre_render_pass_event>(&_buses.render),
	event_listener<shader_use_event>(&_buses.render),
	event_listener<draw_event>(&_buses.render),
	event_listener<program_start_event>(&_buses.lifecycle),
	event_listener<screen_resize_event>(&_buses.render),
	event_listener<keydown_event>(&_buses.input),
	event_listener<keyup_event>(&_buses.input),
	events(_events)
{
	event_listener<pre_render_pass_event>::subscribe();
	event_listener<shader_use_event>::subscribe();
	event_listener<draw_event>::subscribe();
	event_listener<program_start_event>::subscribe();
	event_listener<screen_resize_event>::subscribe();
	event_listener<keydown_event>::subscribe();
	event_listener<keyup_event>::subscribe();

	update_view_mat();
}

int camera_controller::handle(pre_render_pass_event &event) {
	int delta = (int)event.delta.count();

	if (! delta) {
		delta = 1;
	}

	vec3 rel_pos = pos - target;

	if (! is_panning) {
		if (zoom_dir != 0.0f) {
			float zoom = zoom_per_s * ((float)delta) / 1000000.0f;

			rel_pos *= (1.0f + zoom * zoom_dir);
			pos = target + rel_pos;
			view_mat_needs_update = true;
		}

		if (vel_dir != vec3(0.0f)) {
			float half_angle = d_ang_per_s * ((float)delta / 1000000.0f);
			glm::quat total_rot(0.0f, 0.0f, 0.0f, 0.0f);

			if (vel_dir.x != 0.0f) {
				float x_half_angle = half_angle * vel_dir.x;
				vec3 axis = up;

				if (rot_axis != vec3(0.0f)) {
					axis = rot_axis;
				}

				glm::quat x_rot(std::cos(x_half_angle), -std::sin(x_half_angle) * axis);

				total_rot += x_rot;
			}

			if (vel_dir.y != 0.0f && rot_axis == vec3(0.0f)) {
				float y_half_angle = half_angle * vel_dir.y;
				glm::quat y_rot(std::cos(y_half_angle), std::sin(y_half_angle) * right);

				total_rot += y_rot;
			}

			if (vel_dir.z != 0.0f) {
				float z_half_angle = half_angle * vel_dir.z;
				vec3 axis = normalize(target - pos);
				glm::quat z_rot(std::cos(z_half_angle), std::sin(z_half_angle) * axis);

				total_rot += z_rot;
			}

			total_rot = glm::normalize(total_rot);

			rel_pos = total_rot * rel_pos;
			pos = target + rel_pos;
			up = total_rot * up;
			right = total_rot * right;
			view_mat_needs_update = true;
		}
	} else {
		// TODO: vector length function
		float dp_per_s = std::sqrt(dot(rel_pos, rel_pos));
		float dp = dp_per_s * ((float)delta / 1000000.0f);

		if (vel_dir != vec3(0.0f) || zoom_dir != 0.0f) {
			vec3 d_target = dp * (
				-vel_dir.x * right
				- vel_dir.y * up
				+ zoom_dir * normalize(rel_pos)
			);

			target += d_target;
			pos += d_target;
			view_mat_needs_update = true;
		}
	}

	if (view_mat_needs_update) {
		update_view_mat();
		view_mat_needs_update = false;

		camera_move_event move_event(
			pos,
			up,
			right,
			target,
			view,
			inv_view,
			projection
		);
		events.fire(move_event);
	}

	return 0;
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
	update_view_mat();

	camera_move_event move_event(
		pos,
		up,
		right,
		target,
		view,
		inv_view,
		projection
	);
	events.fire(move_event);

	return 0;
}

int camera_controller::handle(screen_resize_event &event) {
	update_projection_mat(event.new_width, event.new_height);

	camera_move_event move_event(
		pos,
		up,
		right,
		target,
		view,
		inv_view,
		projection
	);
	events.fire(move_event);

	return 0;
}

int camera_controller::handle(keydown_event &event) {
	if (event.key == KEY_H) {
		vel_dir.x -= 1.0f;
	} else if (event.key == KEY_J) {
		vel_dir.y -= 1.0f;
	} else if (event.key == KEY_K) {
		vel_dir.y += 1.0f;
	} else if (event.key == KEY_L) {
		vel_dir.x += 1.0f;
	} else if (event.key == KEY_B) {
		vel_dir.z += 1.0f;
	} else if (event.key == KEY_PERIOD) {
		vel_dir.z -= 1.0f;
	} else if (event.key == KEY_I) {
		zoom_dir -= 1.0f;
	} else if (event.key == KEY_O) {
		zoom_dir += 1.0f;
	} else if (event.key == KEY_CTRL) {
		is_panning = true;
	} else if (event.key == KEY_X) {
		rot_axis.x = 1.0f;
		rot_axis = glm::normalize(rot_axis);
	} else if (event.key == KEY_Y) {
		rot_axis.y = 1.0f;
		rot_axis = glm::normalize(rot_axis);
	} else if (event.key == KEY_Z) {
		rot_axis.z = 1.0f;
		rot_axis = glm::normalize(rot_axis);
	}

	return 0;
}

int camera_controller::handle(keyup_event &event) {
	if (event.key == KEY_H) {
		vel_dir.x += 1.0f;
	} else if (event.key == KEY_J) {
		vel_dir.y += 1.0f;
	} else if (event.key == KEY_K) {
		vel_dir.y -= 1.0f;
	} else if (event.key == KEY_L) {
		vel_dir.x -= 1.0f;
	} else if (event.key == KEY_B) {
		vel_dir.z -= 1.0f;
	} else if (event.key == KEY_PERIOD) {
		vel_dir.z += 1.0f;
	} else if (event.key == KEY_I) {
		zoom_dir += 1.0f;
	} else if (event.key == KEY_O) {
		zoom_dir -= 1.0f;
	} else if (event.key == KEY_CTRL) {
		is_panning = false;
	} else if (event.key == KEY_X) {
		rot_axis.x = 0.0f;

		if (rot_axis != vec3(0.0f)) {
			rot_axis = glm::normalize(rot_axis);
		}
	} else if (event.key == KEY_Y) {
		rot_axis.y = 0.0f;

		if (rot_axis != vec3(0.0f)) {
			rot_axis = glm::normalize(rot_axis);
		}
	} else if (event.key == KEY_Z) {
		rot_axis.z = 0.0f;

		if (rot_axis != vec3(0.0f)) {
			rot_axis = glm::normalize(rot_axis);
		}
	}

	return 0;
}

void camera_controller::set_target(const vec3 &new_target) {
	target = new_target;

	assert(target != pos);

	update_dirs();
	view_mat_needs_update = true;
}

void camera_controller::set_pos(const vec3 &new_pos) {
	pos = new_pos;

	assert(target != pos);

	update_dirs();
	view_mat_needs_update = true;
}

void camera_controller::update_projection_mat(int width, int height) {
	projection = glm::perspective(
		glm::radians(60.0f),
		width / (float)height,
		0.1f,
		1000.0f
	);
}

void camera_controller::update_view_mat() {
	view = glm::lookAt(pos, target, up);
	inv_view = glm::inverse(view);
}

void camera_controller::update_dirs() {
	vec3 dir = glm::normalize(target - pos);
	real right_coincidence = std::abs(dot(dir, right));
	real up_coincidence = std::abs(dot(dir, up));

	if (right_coincidence < up_coincidence) {
		right = glm::cross(dir, up);
		assert(right != vec3(0.0f));

		up = glm::cross(right, dir);
		assert(up != vec3(0.0f));
	} else {
		up = glm::cross(right, dir);
		assert(up != vec3(0.0f));

		right = glm::cross(dir, up);
		assert(right != vec3(0.0f));
	}
}