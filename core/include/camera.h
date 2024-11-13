#pragma once
#include "events.h"
#include "shader_program.h"

class camera :
	public event_listener<pre_render_pass_event>,
	public event_listener<shader_use_event>,
	public event_listener<draw_event>,
	public event_listener<screen_resize_event>
{
public:
	glm::vec3 pos;
	glm::vec3 dir;
	glm::vec3 world_up;
	glm::vec3 right;

	camera(event_buses &_buses);

	int handle(pre_render_pass_event &event) override;
	int handle(shader_use_event &event) override;
	int handle(draw_event &event) override;
	int handle(screen_resize_event &event) override;

	void update_right_vec();
	void update_view_mat();
	const glm::mat4& get_view() const;

private:
	glm::vec3 camera_up;
	glm::vec3 target;
	glm::mat4 view;
	glm::mat4 inv_view;
	glm::mat4 projection;
};
