#pragma once
#include <glm/glm.hpp>
#include "event.h"
#include "instanced_mesh.h"
#include "physics/particle.h"

class tool;

struct tool_register_event {
	const tool * t;

	tool_register_event(const tool * _tool) : t(_tool) {}
};

struct tool_select_event {
	const tool * t;

	tool_select_event(const tool * _tool) : t(_tool) {}
};

struct particle_spawn_event {
	const glm::vec3 pos;

	particle_spawn_event(const glm::vec3 &_pos) : pos(_pos) {}
};

struct rod_spawn_event {
	const size_t particle_a_index;
	const size_t particle_b_index;

	rod_spawn_event(
		size_t _particle_a_index,
		size_t _particle_b_index
	) :
		particle_a_index(_particle_a_index),
		particle_b_index(_particle_b_index)
	{}
};

struct cable_spawn_event {
	const size_t particle_a_index;
	const size_t particle_b_index;

	cable_spawn_event(
		size_t _particle_a_index,
		size_t _particle_b_index
	) :
		particle_a_index(_particle_a_index),
		particle_b_index(_particle_b_index)
	{}
};

struct particle_select_event {
	instanced_mesh &particle_mesh;
	phys::particle &p;
	size_t particle_index;

	particle_select_event(
		instanced_mesh &_particle_mesh,
		phys::particle &_p,
		size_t _particle_index
	) :
		particle_mesh(_particle_mesh),
		p(_p),
		particle_index(_particle_index)
	{}
};

struct particle_deselect_event {
	instanced_mesh &particle_mesh;

	particle_deselect_event(
		instanced_mesh &_particle_mesh
	) :
		particle_mesh(_particle_mesh)
	{}
};

using custom_event_bus = event_bus<
	tool_register_event,
	tool_select_event,
	particle_spawn_event,
	rod_spawn_event,
	cable_spawn_event,
	particle_select_event,
	particle_deselect_event
>;
