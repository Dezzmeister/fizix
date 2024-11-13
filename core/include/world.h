#pragma once
#include <functional>
#include "events.h"
#include "instanced_mesh.h"
#include "light.h"
#include "mesh.h"
#include "particle_emitter.h"
#include "rendering.h"

class world :
	public event_listener<pre_render_pass_event>,
	public event_listener<draw_event>,
	public event_listener<screen_resize_event>,
	public event_listener<program_start_event>,
	public event_listener<player_spawn_event>,
	public event_listener<player_move_event>
{
public:
	world(event_buses &_buses, std::vector<mesh *> _meshes = {}, std::vector<light *> _lights = {});

	int handle(pre_render_pass_event &event) override;
	int handle(draw_event &event) override;
	int handle(screen_resize_event &event) override;
	int handle(program_start_event &event) override;
	int handle(player_spawn_event &event) override;
	int handle(player_move_event &event) override;

	// Meshes are sorted in ascending order
	void add_mesh(mesh * m);
	void add_mesh_unsorted(mesh * m);
	void remove_mesh(const mesh * m);

	void add_light(light * l);
	void remove_light(const light * l);

	void add_instanced_mesh(instanced_mesh * _mesh);
	void remove_instanced_mesh(const instanced_mesh * _mesh);

	void add_particle_emitter(particle_emitter * emitter);
	void remove_particle_emitter(particle_emitter * emitter);

	const std::vector<light *>& get_lights() const;

private:
	event_buses &buses;
	std::vector<mesh *> meshes{};
	std::vector<light *> lights{};
	std::vector<mesh *> transparent_meshes{};
	bool meshes_need_sorting{ false };
	std::vector<instanced_mesh *> instanced_meshes{};
	std::vector<particle_emitter *> particle_emitters{};
	// TODO: Remove these dimensions
	int screen_width{};
	int screen_height{};
	int max_tex_units{ -1 };
	int default_sampler2d_tex_unit{ -1 };
	int default_cubesampler_tex_unit{ -1 };
	glm::vec3 player_pos{};
	const std::function<bool(const mesh *, const mesh *)> transparent_mesh_cmp;

	void prepare_draw_lights(const shader_program &shader, render_pass_state &render_pass) const;
	void prepare_shadow_maps(draw_event &event) const;

	void draw_meshes(draw_event &event) const;
	void draw_transparent_meshes(draw_event &event) const;
	void draw_instanced_meshes(draw_event &event) const;
	void draw_particles(draw_event &event) const;

	void draw_meshes(draw_event &event, const std::vector<mesh *> &_meshes, const std::string &shader_modifier = "") const;
};
