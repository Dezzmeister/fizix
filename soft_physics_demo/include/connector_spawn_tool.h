#pragma once
#include <memory>
#include "events.h"
#include "instanced_mesh.h"
#include "physics/particle.h"
#include "custom_events.h"
#include "tools.h"

enum class connector_type {
	Rod,
	Cable
};

class connector_spawn_tool :
	public tool,
	public event_listener<pre_render_pass_event>,
	public event_listener<mousedown_event>,
	public event_listener<particle_select_event>,
	public event_listener<particle_deselect_event>
{
public:
	connector_spawn_tool(
		event_buses &_buses,
		custom_event_bus &_custom_bus,
		texture_store &_textures,
		world &_mesh_world,
		connector_type _type
	);

	void activate() override;
	void deactivate() override;
	bool is_active() const override;

	int handle(pre_render_pass_event &event) override;
	int handle(mousedown_event &event) override;
	int handle(particle_select_event &event) override;
	int handle(particle_deselect_event &event) override;

private:
	event_buses &buses;
	custom_event_bus &custom_bus;
	world &mesh_world;
	connector_type type;
	const phys::particle * selected_particle{ nullptr };
	int64_t selected_particle_index{ -1 };
	const phys::particle * particle_a{ nullptr };
	const phys::particle * particle_b{ nullptr };
	int64_t particle_a_index{ -1 };
	int64_t particle_b_index{ -1 };
	std::unique_ptr<mesh> selected_a_mesh;
	instanced_mesh * particle_mesh{ nullptr };
};
