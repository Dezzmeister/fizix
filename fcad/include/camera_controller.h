#pragma once
#include <events.h>
#include "fcad_events.h"

class camera_controller :
	public event_listener<shader_use_event>,
	public event_listener<draw_event>,
	public event_listener<program_start_event>,
	public event_listener<screen_resize_event>,
	public event_listener<camera_move_event>
{
public:
	camera_controller(event_buses &_buses, fcad_event_bus &_events);

	int handle(shader_use_event &event) override;
	int handle(draw_event &event) override;
	int handle(program_start_event &event) override;
	int handle(screen_resize_event &event) override;
	int handle(camera_move_event &event) override;

private:
	glm::vec3 pos{ 0.0f, 0.0f, 1.0f };
	glm::vec3 up{ 0.0f, 1.0f, 0.0f };
	glm::vec3 right{ 1.0f, 0.0f, 0.0f };
	glm::mat4 view{};
	glm::mat4 inv_view{};
	glm::mat4 projection{};
	float angle_step{ 0.01f };

	void update_projection_mat(int width, int height);
	void update_view_mat();
};