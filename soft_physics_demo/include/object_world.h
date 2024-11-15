#pragma once
#include <array>
#include <bitset>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include "events.h"
#include "instanced_mesh.h"
#include "logging.h"
#include "phong_color_material.h"
#include "physics/constraints.h"
#include "physics/particle.h"
#include "physics/particle_force_generators.h"
#include "physics/particle_world.h"
#include "shapes.h"
#include "world.h"
#include "constants.h"
#include "custom_events.h"
#include "particle_collision_constraint_generator.h"
#include "raycast.h"

using namespace phys::literals;

namespace {
	// Cyan plastic
	phong_color_material rod_mtl{
		phong_color_material_properties{
			glm::vec3(0.0, 0.1, 0.06),
			glm::vec3(0.0, 0.50980392, 0.50980392),
			glm::vec3(0.50196078, 0.50196078, 0.50196078),
			128 * 0.25f
		}
	};

	// Obsidian
	phong_color_material floor_mtl{
		phong_color_material_properties{
			glm::vec3(0.05375, 0.05, 0.06625),
			glm::vec3(0.18275, 0.17, 0.22525),
			glm::vec3(0.332741, 0.328634, 0.346435),
			128 * 0.3f
		},
	};

	// Black rubber
	phong_color_material cable_mtl{
		phong_color_material_properties{
			glm::vec3(0.02, 0.02, 0.02),
			glm::vec3(0.01, 0.01, 0.01),
			glm::vec3(0.4, 0.4, 0.4),
			128 * 0.078125f
		},
	};

	constexpr float floor_y = -1.0f;
	constexpr float rod_radius = 0.05f;
	constexpr glm::vec3 y_axis = glm::vec3(0.0f, 1.0f, 0.0f);

	constexpr phys::real cable_segment_length = 0.1_r;
	constexpr float cable_radius = 0.01f;

	struct cable {
		std::vector<phys::particle> particles{};
		std::vector<phys::distance_constraint> pieces{};
		phys::plane_collision_constraint_generator<std::vector<phys::particle>> floor_constraint_generator;
		size_t cable_mesh_offset{};

		cable();
	};

	template <const size_t N>
	struct world_state {
		geometry rod_geom;
		instanced_mesh sphere_meshes;
		instanced_mesh rod_meshes;
		instanced_mesh cable_meshes;

		std::array<phys::particle, N> particles{};
		std::bitset<N> active{};
		// These won't be moved, because we reserve N of them on construction
		std::vector<phys::distance_constraint> rods{};
		// Same with these - we reserve N
		std::vector<std::unique_ptr<cable>> cables{};

		world_state(
			world &_mesh_world,
			geometry _rod_geom,
			const glm::vec3 &_hiding_pos
		);

		void update_meshes();

		void hide_object(instanced_mesh &m, size_t i);

		// Returns true if the selection was successful (i.e. it changed
		// state), false if not.
		bool select_particle(int64_t i);

		phys::distance_constraint * create_rod(size_t particle_a_index, size_t particle_b_index);

		cable * create_cable(size_t particle_a_index, size_t particle_b_index);

	private:
		static constexpr size_t max_cable_segments = N * 10;

		// Any unused instanced mesh instances will be moved to a location
		// far from anything that the player is likely to encounter
		glm::mat4 hiding_transform;
		int64_t selected_particle{ -1 };
		world &mesh_world;

		glm::mat4 particle_transform_mat(size_t i) const;

		size_t used_cable_segments() const;
		size_t next_cable_mesh_offset() const;
	};

	// A raycast hit result
	struct hit_result {
		size_t i{ 0 };
		phys::real t{ -1.0_r };
	};
}

template <const size_t N>
class object_world :
	public event_listener<pre_render_pass_event>,
	public event_listener<particle_spawn_event>,
	public event_listener<rod_spawn_event>,
	public event_listener<cable_spawn_event>,
	public event_listener<keydown_event>,
	public event_listener<player_spawn_event>,
	public event_listener<player_move_event>,
	public event_listener<player_look_event>
{
public:
	object_world(
		event_buses &_buses,
		custom_event_bus &_custom_bus,
		world &_mesh_world,
		short _pause_key,
		short _step_key
	);

	int handle(pre_render_pass_event &event) override;
	int handle(particle_spawn_event &event) override;
	int handle(rod_spawn_event &event) override;
	int handle(cable_spawn_event &event) override;
	int handle(keydown_event &event) override;
	int handle(player_spawn_event &event) override;
	int handle(player_move_event &event) override;
	int handle(player_look_event &event) override;

private:
	std::unique_ptr<world_state<N>> state;
	world &mesh_world;
	phys::particle_world phys_world;
	custom_event_bus &custom_bus;

	std::unique_ptr<directional_light> light{};
	std::unique_ptr<mesh> floor{};

	std::unique_ptr<phys::plane_collision_constraint_generator<std::array<phys::particle, N>>> floor_constraint_generator;
	std::unique_ptr<particle_collision_constraint_generator<N>> particle_collision_generator;
	std::unique_ptr<phys::particle_gravity> gravity_generator;
	size_t phys_iter_per_s;
	long long timestep_ms;
	phys::real timestep_s;
	long long prev_update{};

	glm::vec3 player_pos{};
	glm::vec3 player_dir{};
	std::vector<hit_result> hit_results{};

	short pause_key;
	short step_key;
	bool paused{ false };
	bool step{ false };

	int64_t next_inactive_particle() const;
	void do_raycast_and_update();
};

template <const size_t N>
world_state<N>::world_state(
	world &_mesh_world,
	geometry _rod_geom,
	const glm::vec3 &_hiding_pos
) :
	rod_geom(std::move(_rod_geom)),
	sphere_meshes(sphere_geom.get(), &sphere_mtl, N),
	rod_meshes(&rod_geom, &rod_mtl, N),
	cable_meshes(&rod_geom, &cable_mtl, max_cable_segments),
	hiding_transform(glm::translate(glm::identity<glm::mat4>(), _hiding_pos)),
	mesh_world(_mesh_world)
{
	for (size_t i = 0; i < N; i++) {
		hide_object(sphere_meshes, i);
		hide_object(rod_meshes, i);
	}

	for (size_t i = 0; i < max_cable_segments; i++) {
		hide_object(cable_meshes, i);
	}

	mesh_world.add_instanced_mesh(&sphere_meshes);
	mesh_world.add_instanced_mesh(&rod_meshes);
	mesh_world.add_instanced_mesh(&cable_meshes);

	rods.reserve(N);
	cables.reserve(N);
}

template <const size_t N>
void world_state<N>::update_meshes() {
	for (size_t i = 0; i < N; i++) {
		if (active[i]) {
			sphere_meshes.set_model(i, particle_transform_mat(i));
		}
	}

	for (size_t i = 0; i < rods.size(); i++) {
		phys::distance_constraint &rod = rods[i];

		glm::vec3 dr = phys::to_glm<glm::vec3>(rod.a()->pos - rod.b()->pos);
		float r = glm::length(dr);

		glm::mat4 scale_mat = glm::scale(
			glm::identity<glm::mat4>(),
			glm::vec3(rod_radius, r, rod_radius)
		);

		glm::vec3 axis = glm::cross(y_axis, dr);
		glm::quat rot = glm::identity<glm::quat>();

		if (r != 0.0f && glm::dot(axis, axis) != 0.0f) {
			float cos_t = glm::dot(y_axis, dr) / r;
			float cos_t_2 = std::sqrt((1.0f + cos_t) / 2.0f);
			float sin_t_2 = std::sqrt((1.0f - cos_t) / 2.0f);

			rot = glm::quat(cos_t_2, sin_t_2 * glm::normalize(axis));
		}

		glm::mat4 rot_mat = glm::mat4_cast(rot);

		glm::mat4 trans_mat = glm::translate(
			glm::identity<glm::mat4>(),
			(rod.a()->pos + rod.b()->pos) / 2.0f
		);

		rod_meshes.set_model(i, trans_mat * rot_mat * scale_mat);
	}

	for (size_t i = 0; i < cables.size(); i++) {
		cable &c = *cables[i];

		for (size_t j = 0; j < c.pieces.size(); j++) {
			phys::distance_constraint &cable = c.pieces[j];

			glm::vec3 dr = phys::to_glm<glm::vec3>(cable.a()->pos - cable.b()->pos);
			float r = glm::length(dr);

			glm::mat4 scale_mat = glm::scale(
				glm::identity<glm::mat4>(),
				glm::vec3(cable_radius, r, cable_radius)
			);

			glm::vec3 axis = glm::cross(y_axis, dr);
			glm::quat rot = glm::identity<glm::quat>();

			if (r != 0.0f && glm::dot(axis, axis) != 0.0f) {
				float cos_t = glm::dot(y_axis, dr) / r;
				float cos_t_2 = std::sqrt((1.0f + cos_t) / 2.0f);
				float sin_t_2 = std::sqrt((1.0f - cos_t) / 2.0f);

				rot = glm::quat(cos_t_2, sin_t_2 * glm::normalize(axis));
			}

			glm::mat4 rot_mat = glm::mat4_cast(rot);

			glm::mat4 trans_mat = glm::translate(
				glm::identity<glm::mat4>(),
				(cable.a()->pos + cable.b()->pos) / 2.0f
			);

			cable_meshes.set_model(j + c.cable_mesh_offset, trans_mat * rot_mat * scale_mat);
		}
	}
}

template <const size_t N>
void world_state<N>::hide_object(instanced_mesh &m, size_t i) {
	m.set_model(i, hiding_transform);
}

template <const size_t N>
bool world_state<N>::select_particle(int64_t i) {
	if (selected_particle == i) {
		return false;
	}

	selected_particle = i;

	return true;
}

template <const size_t N>
glm::mat4 world_state<N>::particle_transform_mat(size_t i) const {
	return glm::translate(
		glm::identity<glm::mat4>(),
		phys::to_glm<glm::vec3>(particles[i].pos)
	) * sphere_scale;
}

template <const size_t N>
phys::distance_constraint * world_state<N>::create_rod(size_t particle_a_index, size_t particle_b_index) {
	phys::particle * a = &particles[particle_a_index];
	phys::particle * b = &particles[particle_b_index];
	phys::real length = std::sqrt(phys::dot(a->pos - b->pos, a->pos - b->pos));

	phys::distance_constraint rod(a, b, length, 1.0_r);
	rods.push_back(rod);

	return &rods[rods.size() - 1];
}

cable::cable() :
	floor_constraint_generator(
		particles,
		phys::vec3(0.0_r, 1.0_r, 0.0_r),
		phys::vec3(0.0_r, floor_y + cable_radius, 0.0_r),
		0.9_r,
		0.6_r
	)
{}

template <const size_t N>
cable * world_state<N>::create_cable(size_t particle_a_index, size_t particle_b_index) {
	phys::particle * a = &particles[particle_a_index];
	phys::particle * b = &particles[particle_b_index];

	phys::vec3 r = b->pos - a->pos;
	phys::real d = std::sqrt(phys::dot(r, r));

	size_t segments_needed = (size_t)std::ceil(d / cable_segment_length);
	size_t used_segments = used_cable_segments();

	if (used_segments + segments_needed > max_cable_segments) {
		// TODO: Show the user an error
		return nullptr;
	}

	std::unique_ptr<cable> out = std::make_unique<cable>();
	out->cable_mesh_offset = next_cable_mesh_offset();

	if (segments_needed == 1) {
		phys::distance_constraint segment(a, b, d, 1.0_r);

		out->particles = {};
		out->pieces = { segment };
		cables.push_back(std::move(out));

		return cables[cables.size() - 1].get();
	}

	const phys::real step = d / segments_needed;
	constexpr phys::real linear_mass_density = 0.5_r /* kg/m */;
	const phys::real mass = linear_mass_density * step;

	for (size_t i = 0; i < segments_needed - 1; i++) {
		phys::particle p{};
		p.pos = a->pos + (((i + 1) / (phys::real)segments_needed) * r);
		p.vel = phys::vec3(0.0_r);
		p.acc = phys::vec3(0.0_r);
		p.force = phys::vec3(0.0_r);
		p.damping = 0.995_r;
		p.set_mass(mass);

		out->particles.push_back(p);
	}

	phys::distance_constraint first_piece(a, &out->particles[0], step, 1.0_r);
	phys::distance_constraint last_piece(&out->particles[out->particles.size() - 1], b, step, 1.0_r);

	out->pieces.push_back(first_piece);

	for (size_t i = 0; i < out->particles.size() - 1; i++) {
		phys::distance_constraint piece(&out->particles[i], &out->particles[i + 1], step, 1.0_r);
		out->pieces.push_back(piece);
	}

	out->pieces.push_back(last_piece);

	cables.push_back(std::move(out));

	return cables[cables.size() - 1].get();
}

template <const size_t N>
size_t world_state<N>::used_cable_segments() const {
	size_t out = 0;

	for (size_t i = 0; i < cables.size(); i++) {
		out += cables[i]->pieces.size();
	}

	return out;
}

template <const size_t N>
size_t world_state<N>::next_cable_mesh_offset() const {
	if (! cables.size()) {
		return 0;
	}

	// TODO: Come up with a better algorithm for this when cables can be deleted
	const cable &c = *cables[cables.size() - 1];

	return c.cable_mesh_offset + c.pieces.size();
}

template <const size_t N>
object_world<N>::object_world(
	event_buses &_buses,
	custom_event_bus &_custom_bus,
	world &_mesh_world,
	short _pause_key,
	short _step_key
) :
	event_listener<pre_render_pass_event>(&_buses.render, -10),
	event_listener<particle_spawn_event>(&_custom_bus),
	event_listener<rod_spawn_event>(&_custom_bus),
	event_listener<cable_spawn_event>(&_custom_bus),
	event_listener<keydown_event>(&_buses.input),
	event_listener<player_spawn_event>(&_buses.player),
	event_listener<player_move_event>(&_buses.player),
	event_listener<player_look_event>(&_buses.player),
	state(std::make_unique<world_state<N>>(
		_mesh_world,
		shapes::make_cylinder(20, true),
		glm::vec3(0.0f, -10000.0f, 0.0f)
	)),
	mesh_world(_mesh_world),
	phys_world(16),
	custom_bus(_custom_bus),
	floor_constraint_generator(std::make_unique<phys::plane_collision_constraint_generator<std::array<phys::particle, N>>>(
		state->particles,
		phys::vec3(0.0_r, 1.0_r, 0.0_r),
		phys::vec3(0.0_r, floor_y + sphere_radius, 0.0_r),
		0.9_r,
		0.6_r
	)),
	particle_collision_generator(std::make_unique<particle_collision_constraint_generator<N>>(
		state->particles,
		state->active
	)),
	gravity_generator(std::make_unique<phys::particle_gravity>(
		phys::vec3(0.0_r, -9.8_r, 0.0_r)
	)),
	phys_iter_per_s(60),
	timestep_ms(1000 / phys_iter_per_s),
	timestep_s(timestep_ms / 1000.0_r),
	pause_key(_pause_key),
	step_key(_step_key)
{
	event_listener<pre_render_pass_event>::subscribe();
	event_listener<particle_spawn_event>::subscribe();
	event_listener<rod_spawn_event>::subscribe();
	event_listener<cable_spawn_event>::subscribe();
	event_listener<keydown_event>::subscribe();
	event_listener<player_spawn_event>::subscribe();
	event_listener<player_move_event>::subscribe();
	event_listener<player_look_event>::subscribe();

	light = std::make_unique<directional_light>(
		glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f)),
		light_properties(
			glm::vec3(1.0f),
			glm::vec3(1.0f),
			glm::vec3(1.0f)
		),
		directional_shadow_caster_properties(
			glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f)),
			10.0f,
			100.0f,
			2048
		)
	);

	light->set_casts_shadow(true);

	mesh_world.add_light(light.get());

	floor = std::make_unique<mesh>(shapes::plane.get(), &floor_mtl);
	floor->set_model(glm::translate(glm::identity<glm::mat4>(), glm::vec3(
		0.0f,
		floor_y,
		0.0f
	)) * glm::scale(glm::identity<glm::mat4>(), glm::vec3(100.0f, 1.0f, 100.0f)));

	mesh_world.add_mesh(floor.get());

	phys_world.add_constraint_generator(floor_constraint_generator.get());
	phys_world.add_constraint_generator(particle_collision_generator.get());
}

template <const size_t N>
int object_world<N>::handle(pre_render_pass_event&) {
	std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
	std::chrono::time_point<std::chrono::steady_clock, std::chrono::milliseconds> now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
	long long millis = now_ms.time_since_epoch().count();

	if (! prev_update) {
		prev_update = millis;
		return 0;
	}

	if (! paused || (paused && step)) {
		long long dm = millis - prev_update;
		long long num_iters = dm / timestep_ms;

		if (num_iters * timestep_ms > 1000) {
			num_iters = 1;
		}

		if (num_iters) {
			step = false;
			prev_update += (num_iters * timestep_ms);
		}

		for (long long i = 0; i < num_iters; i++) {
			phys_world.prepare_frame();
			phys_world.run_physics(timestep_s);
		}
	}

	do_raycast_and_update();
	state->update_meshes();

	return 0;
}

template <const size_t N>
int object_world<N>::handle(particle_spawn_event &event) {
	int64_t i = next_inactive_particle();

	if (i == -1) {
		logger::warn("Max particles reached (" + traits::to_string(N) + ")");
		// TODO: Do something in the game world
		return 1;
	}

	state->particles[i].pos = event.pos;
	state->particles[i].radius = sphere_radius;
	state->active[i] = true;
	phys_world.add_particle(&state->particles[i]);
	phys_world.force_registry.add(&state->particles[i], gravity_generator.get());

	return 0;
}

template <const size_t N>
int object_world<N>::handle(rod_spawn_event &event) {
	phys::distance_constraint * rod = state->create_rod(event.particle_a_index, event.particle_b_index);
	phys_world.add_fixed_constraint(rod);

	return 0;
}

template <const size_t N>
int object_world<N>::handle(cable_spawn_event &event) {
	cable * c = state->create_cable(event.particle_a_index, event.particle_b_index);

	for (phys::particle &p : c->particles) {
		phys_world.add_particle(&p);
		phys_world.force_registry.add(&p, gravity_generator.get());
	}

	for (phys::distance_constraint &rod : c->pieces) {
		phys_world.add_fixed_constraint(&rod);
	}

	phys_world.add_constraint_generator(&c->floor_constraint_generator);

	return 0;
}

template <const size_t N>
int object_world<N>::handle(keydown_event &event) {
	if (! event.is_mouse_locked) {
		return 0;
	}

	if (event.key == pause_key) {
		paused = ! paused;
	} else if (event.key == step_key) {
		step = true;
	}

	return 0;
}

template <const size_t N>
int object_world<N>::handle(player_spawn_event &event) {
	player_pos = event.pos;
	player_dir = event.dir;
	light->shadow_props.set_eye_pos(event.pos + glm::vec3(0.0f, 20.0f, 0.0f));

	return 0;
}

template <const size_t N>
int object_world<N>::handle(player_move_event &event) {
	player_pos = event.pos;
	light->shadow_props.set_eye_pos(event.pos + glm::vec3(0.0f, 20.0f, 0.0f));

	return 0;
}

template <const size_t N>
int object_world<N>::handle(player_look_event &event) {
	player_dir = event.dir;

	return 0;
}

template <const size_t N>
int64_t object_world<N>::next_inactive_particle() const {
	for (size_t i = 0; i < N; i++) {
		if (! state->active[i]) {
			return i;
		}
	}

	return -1;
}

template <const size_t N>
void object_world<N>::do_raycast_and_update() {
	hit_results.clear();

	for (size_t i = 0; i < N; i++) {
		if (! state->active[i]) {
			continue;
		}

		phys::real t = raycast_sphere_test(player_pos, player_dir, state->particles[i].pos, state->particles[i].radius);

		if (t >= 0.0_r) {
			hit_results.push_back({
				.i = i,
				.t = t
				});
		}
	}

	if (! hit_results.size()) {
		bool was_selected = state->select_particle(-1);

		if (was_selected) {
			particle_deselect_event deselect_event(state->sphere_meshes);
			custom_bus.fire(deselect_event);
		}

		return;
	}

	phys::real min_t = hit_results[0].t;
	size_t min_i = hit_results[0].i;

	for (size_t i = 1; i < hit_results.size(); i++) {
		if (hit_results[i].t < min_t) {
			min_t = hit_results[i].t;
			min_i = hit_results[i].i;
		}
	}

	bool was_selected = state->select_particle(min_i);

	if (was_selected) {
		particle_select_event select_event(state->sphere_meshes, state->particles[min_i], min_i);
		custom_bus.fire(select_event);
	}
}
