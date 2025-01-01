#pragma once
#include <events.h>
#include "fcad_events.h"

class camera_controller :
	traits::pinned<camera_controller>,
	public event_listener<pre_render_pass_event>,
	public event_listener<shader_use_event>,
	public event_listener<draw_event>,
	public event_listener<program_start_event>,
	public event_listener<screen_resize_event>,
	public event_listener<keydown_event>,
	public event_listener<keyup_event>
{
public:
	camera_controller(event_buses &_buses, fcad_event_bus &_events);

	void set_target(const vec3 &new_target);
	void set_pos(const vec3 &new_pos);

	int handle(pre_render_pass_event &event) override;
	int handle(shader_use_event &event) override;
	int handle(draw_event &event) override;
	int handle(program_start_event &event) override;
	int handle(screen_resize_event &event) override;
	int handle(keydown_event &event) override;
	int handle(keyup_event &event) override;

private:
	fcad_event_bus &events;
	glm::vec3 pos{ 0.0f, 0.0f, 10.0f };
	glm::vec3 up{ 0.0f, 1.0f, 0.0f };
	glm::vec3 right{ 1.0f, 0.0f, 0.0f };
	glm::vec3 target{};
	glm::mat4 view{};
	glm::mat4 inv_view{};
	glm::mat4 projection{};
	glm::vec3 vel_dir{};
	glm::vec3 rot_axis{};
	float d_ang_per_s{ 2.0f };
	float zoom_dir{};
	float zoom_per_s{ 0.92f };
	bool is_panning{};
	bool view_mat_needs_update{};

	void update_projection_mat(int width, int height);
	void update_view_mat();
	void update_dirs();
};