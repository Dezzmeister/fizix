#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#define _USE_MATH_DEFINES
#include <math.h>
#include "player.h"

player::player(event_buses &_buses) :
	event_listener<keydown_event>(&_buses.input),
	event_listener<keyup_event>(&_buses.input),
	event_listener<pre_render_pass_event>(&_buses.render, -50),
	event_listener<mouse_lock_event>(&_buses.input),
	event_listener<mouse_unlock_event>(&_buses.input),
	event_listener<mouse_move_event>(&_buses.input),
	speed(2.0f),
	look_sensitivity(0.5f),
	buses(_buses),
	cam(buses),
	input_vel(0.0f, 0.0f),
	sprint_mul(1.0f)
{
	cam.dir = glm::vec3(0.0f, 0.0f, 1.0f);

	event_listener<keydown_event>::subscribe();
	event_listener<keyup_event>::subscribe();
	event_listener<pre_render_pass_event>::subscribe();
	event_listener<mouse_lock_event>::subscribe();
	event_listener<mouse_unlock_event>::subscribe();
	event_listener<mouse_move_event>::subscribe();
}

int player::handle(keydown_event &event) {
	if (event.key == KEY_W) {
		input_vel.y += 1.0f;
	} else if (event.key == KEY_S) {
		input_vel.y -= 1.0f;
	} else if (event.key == KEY_A) {
		input_vel.x -= 1.0f;
	} else if (event.key == KEY_D) {
		input_vel.x += 1.0f;
	} else if (event.key == KEY_SHIFT) {
		sprint_mul = 1.5f;
	}

	return 0;
}

int player::handle(keyup_event &event) {
	if (event.key == KEY_W) {
		input_vel.y -= 1.0f;
	} else if (event.key == KEY_S) {
		input_vel.y += 1.0f;
	} else if (event.key == KEY_A) {
		input_vel.x += 1.0f;
	} else if (event.key == KEY_D) {
		input_vel.x -= 1.0f;
	} else if (event.key == KEY_SHIFT) {
		sprint_mul = 1.0f;
	}

	return 0;
}

int player::handle(pre_render_pass_event &event) {
	if (! sent_spawn_event) {
		player_spawn_event spawn_event(*this, cam.pos, cam.dir);
		buses.player.fire(spawn_event);
		sent_spawn_event = true;
	}

	if (input_vel.y > 1.0f || input_vel.y < -1.0f) {
		input_vel.y = 0;
	}

	if (input_vel.x > 1.0f || input_vel.x < -1.0f) {
		input_vel.x = 0;
	}

	// Velocity clamp
	const float input_vel_len = glm::length(input_vel);
	const float vel_norm_f = (input_vel_len > 1.0f ? input_vel_len : 1.0f);
	const long long delta = event.delta.count() == 0 ? 1 : event.delta.count();
	// Factor by which to adjust speed based on delta
	const float delta_f = ((float)delta) / 1000.0f;
	const float full_speed_f = speed * sprint_mul * delta_f / vel_norm_f;
	const float move_y = input_vel.y * full_speed_f;
	const float move_x = input_vel.x * full_speed_f;

	if (is_mouse_locked) {
		glm::vec3 old_dir = cam.dir;

		float dx = (float)(delta_mouse.x) * look_sensitivity;
		float dy = (float)(delta_mouse.y) * look_sensitivity;
		delta_mouse = {};

		float yaw_angle = -(float)(dx * M_PI / 180.0);
		glm::quat yaw(cosf(yaw_angle / 2), cam.world_up * sinf(yaw_angle / 2));

		cam.dir = glm::normalize(yaw * cam.dir);

		cam.update_right_vec();

		// TODO: Clean this stuff up
		constexpr float offset = 0.3f;
		glm::quat top_offset_rot(-cosf(offset / 2), cam.right * sinf(offset / 2));
		glm::vec3 top_max_dir = top_offset_rot * cam.world_up;
		glm::quat bot_offset_rot(cosf(offset / 2), cam.right * sinf(offset / 2));
		glm::vec3 bot_max_dir = bot_offset_rot * -cam.world_up;

		float pitch_angle = (float)std::clamp((dy * M_PI / 180.0), -M_PI / 2 + offset, M_PI / 2 - offset);
		glm::quat pitch(cosf(pitch_angle / 2), cam.right * sinf(pitch_angle / 2));

		glm::vec3 pot_dir = glm::normalize(pitch * cam.dir);

		// Take some cross products to figure out if the new dir would be behind the top and
		// bottom dirs
		glm::vec3 top_dir_right = glm::normalize(glm::cross(cam.dir, top_max_dir));
		glm::vec3 top_pot_dir_right = glm::normalize(glm::cross(pot_dir, top_max_dir));
		glm::vec3 bot_dir_right = glm::normalize(glm::cross(cam.dir, bot_max_dir));
		glm::vec3 bot_pot_dir_right = glm::normalize(glm::cross(pot_dir, bot_max_dir));
		float top_coincidence = glm::dot(top_dir_right, top_pot_dir_right);
		float bot_coincidence = glm::dot(bot_dir_right, bot_pot_dir_right);

		if (((1 - top_coincidence) <= 1e-6f) && ((1 - bot_coincidence) <= 1e-6f)) {
			cam.dir = pot_dir;
		}

		if (cam.dir != old_dir) {
			player_look_event ple(*this, cam.pos, cam.dir, old_dir, cam.get_view());
			buses.player.fire(ple);
		}

		cam.update_view_mat();
	}

	glm::vec3 proj_dir = glm::vec3(cam.dir);
	proj_dir.y = 0;
	proj_dir = glm::normalize(proj_dir);

	glm::vec3 proj_right = glm::vec3(cam.right);
	proj_right.y = 0;
	proj_right = glm::normalize(proj_right);

	glm::vec3 pos_diff = (move_y * proj_dir) + (-move_x * proj_right);

	if (pos_diff.x == 0.0f && pos_diff.y == 0.0f && pos_diff.z == 0.0f) {
		return 0;
	}

	if (is_mouse_locked) {
		player_move_event pme(*this, cam.pos + pos_diff, cam.pos, cam.dir);
		buses.player.fire(pme);
		cam.pos = pme.pos;
	}

	cam.update_view_mat();

	return 0;
}

int player::handle(mouse_lock_event &) {
	is_mouse_locked = true;

	return 0;
}

int player::handle(mouse_unlock_event&) {
	is_mouse_locked = false;

	return 0;
}

int player::handle(mouse_move_event &event) {
	delta_mouse.x = event.delta_x;
	delta_mouse.y = event.delta_y;

	return 0;
}

const camera& player::get_camera() const {
	return cam;
}
