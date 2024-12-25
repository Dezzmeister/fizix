#include <shader_constants.h>
#include <util.h>
#include "camera_controller.h"

camera_controller::camera_controller(
	event_buses &_buses,
	fcad_event_bus&
) :
	event_listener<pre_render_pass_event>(&_buses.render),
	event_listener<shader_use_event>(&_buses.render),
	event_listener<draw_event>(&_buses.render),
	event_listener<program_start_event>(&_buses.lifecycle),
	event_listener<screen_resize_event>(&_buses.render),
	event_listener<keydown_event>(&_buses.input),
	event_listener<keyup_event>(&_buses.input)
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
	// TODO: Finer-resolution deltas
	int delta = event.delta.count() || 1;
	bool view_mat_needs_update = false;

	if (zoom_dir != 0.0f) {
		float zoom = zoom_per_s * ((float)delta) / 1000.0f;

		pos *= (1.0f + zoom * zoom_dir);
		view_mat_needs_update = true;
	}

	if (vel_dir != vec2(0.0f)) {
		float half_angle = d_ang_per_s * ((float)delta) / 1000.0f;
		glm::quat total_rot(0.0f, 0.0f, 0.0f, 0.0f);

		if (vel_dir.x != 0.0f) {
			float x_half_angle = half_angle * vel_dir.x;
			glm::quat x_rot(std::cos(x_half_angle), -std::sin(x_half_angle) * up);

			total_rot += x_rot;
		}

		if (vel_dir.y != 0.0f) {
			float y_half_angle = half_angle * vel_dir.y;
			glm::quat y_rot(std::cos(y_half_angle), std::sin(y_half_angle) * right);

			total_rot += y_rot;
		}

		total_rot = glm::normalize(total_rot);

		pos = total_rot * pos;
		up = total_rot * up;
		right = total_rot * right;
		view_mat_needs_update = true;
	}

	if (view_mat_needs_update) {
		update_view_mat();
		// TODO: Fire an event
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

	return 0;
}

int camera_controller::handle(screen_resize_event &event) {
	update_projection_mat(event.new_width, event.new_height);

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
	} else if (event.key == KEY_I) {
		zoom_dir -= 1.0f;
	} else if (event.key == KEY_O) {
		zoom_dir += 1.0f;
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
	} else if (event.key == KEY_I) {
		zoom_dir += 1.0f;
	} else if (event.key == KEY_O) {
		zoom_dir -= 1.0f;
	}

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