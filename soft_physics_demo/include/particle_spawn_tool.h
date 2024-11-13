#pragma once
#include <memory>
#include "events.h"
#include "mesh.h"
#include "texture_store.h"
#include "world.h"
#include "custom_events.h"
#include "tools.h"

class particle_spawn_tool :
	public tool,
	public event_listener<player_look_event>,
	public event_listener<player_move_event>,
	public event_listener<player_spawn_event>,
	public event_listener<mousedown_event>,
	public event_listener<mouseup_event>,
	public event_listener<mouse_scroll_event>,
	public event_listener<pre_render_pass_event>,
	public event_listener<tool_select_event>
{
public:
	particle_spawn_tool(
		event_buses &_buses,
		custom_event_bus &_custom_bus,
		texture_store &_textures,
		world &_w
	);

	void activate() override;
	void deactivate() override;
	bool is_active() const override;

	int handle(player_look_event &event) override;
	int handle(player_move_event &event) override;
	int handle(player_spawn_event &event) override;
	int handle(mousedown_event &event) override;
	int handle(mouseup_event &event) override;
	int handle(mouse_scroll_event &event) override;
	int handle(pre_render_pass_event &event) override;
	int handle(tool_select_event &event) override;

private:
	custom_event_bus &custom_bus;
	std::unique_ptr<geometry> sphere_geom;
	std::unique_ptr<mesh> preview;
	world &w;
	glm::vec3 dir{ 0.0f };
	glm::vec3 player_pos{ 0.0f };
	bool can_zoom{ false };

	float scroll_f{ 1.0f };
};
