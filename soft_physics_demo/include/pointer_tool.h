#pragma once
#include <unordered_map>
#include "draw2d.h"
#include "texture_store.h"
#include "tools.h"

class pointer_tool :
	public tool,
	public event_listener<program_start_event>,
	public event_listener<pre_render_pass_event>,
	public event_listener<post_processing_event>,
	public event_listener<particle_select_event>,
	public event_listener<particle_deselect_event>,
	public event_listener<mousedown_event>,
	public event_listener<mouseup_event>,
	public event_listener<player_move_event>,
	public event_listener<player_look_event>,
	public event_listener<player_spawn_event>
{
public:
	pointer_tool(
		event_buses &_buses,
		custom_event_bus &_custom_bus,
		texture_store &_textures,
		world &_mesh_world
	);

	void activate() override;
	void deactivate() override;
	bool is_active() const override;

	int handle(program_start_event &event) override;
	int handle(pre_render_pass_event &event) override;
	int handle(post_processing_event &event) override;
	int handle(particle_select_event &event) override;
	int handle(particle_deselect_event &event) override;
	int handle(mousedown_event &event) override;
	int handle(mouseup_event &event) override;
	int handle(player_move_event &event) override;
	int handle(player_look_event &event) override;
	int handle(player_spawn_event &event) override;

private:
	class mesh_updater :
		public event_listener<pre_render_pass_event>
	{
	public:
		phys::particle * selected_particle{};
		instanced_mesh * particle_mesh{};
		mesh * selected_particle_mesh{};
		size_t particle_index{};

		mesh_updater(event_buses &_buses);

		int handle(pre_render_pass_event &event) override;
	};

	phys::particle * selected_particle{};
	size_t particle_index{};
	instanced_mesh * particle_mesh{};
	world &mesh_world;
	std::unique_ptr<mesh> selected_particle_mesh;
	const font * debug_font{};

	glm::vec3 player_pos{};
	glm::vec3 player_dir{};
	phys::particle * held_particle{};
	phys::real held_particle_dist{};
	phys::real held_particle_mass{};
	std::unordered_map<phys::particle *, phys::real> frozen_particles{};
	mesh_updater meshes;
};
