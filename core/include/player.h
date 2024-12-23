#pragma once
#include <functional>
#include "camera.h"
#include "events.h"

class player :
	public event_listener<keydown_event>,
	public event_listener<keyup_event>,
	public event_listener<pre_render_pass_event>,
	public event_listener<mouse_lock_event>,
	public event_listener<mouse_unlock_event>,
	public event_listener<mouse_move_event>
{
public:
	float speed;
	float look_sensitivity;

	player(event_buses &_buses);

	int handle(keydown_event &event) override;
	int handle(keyup_event &event) override;
	int handle(pre_render_pass_event &event) override;
	int handle(mouse_lock_event &event) override;
	int handle(mouse_unlock_event &event) override;
	int handle(mouse_move_event &event) override;

	const camera& get_camera() const;

private:
	event_buses &buses;
	camera cam;
	glm::vec2 input_vel;
	float sprint_mul;
	glm::i64vec2 delta_mouse{};
	bool is_mouse_locked{};
	bool sent_spawn_event{};
};
