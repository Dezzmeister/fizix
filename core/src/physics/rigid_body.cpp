#include <cmath>
#include <stdexcept>
#include "physics/rigid_body.h"

using namespace phys::literals;

phys::rigid_body::rigid_body() :
	linear_damping{ 0.995_r },
	angular_damping{ 0.995_r },
	local_to_world(identity<mat4>()),
	inv_inertia_tensor(identity<mat3>()),
	inv_inertia_tensor_world(identity<mat3>()),
	inv_mass(1.0_r)
{}

void phys::rigid_body::set_mass(real mass) {
	if (mass == infinity) {
		inv_mass = 0.0_r;
		// TODDO: epsilon
	} else if (mass == 0.0_r) {
		throw std::invalid_argument("Mass must be nonzero");
	} else {
		inv_mass = 1.0_r / mass;
	}
}

phys::real phys::rigid_body::get_mass() const {
	// TODO: epsilon
	if (inv_mass == 0.0_r) {
		return infinity;
	}

	return 1.0_r / inv_mass;
}

phys::real phys::rigid_body::get_inv_mass() const {
	return inv_mass;
}

bool phys::rigid_body::has_finite_mass() const {
	return inv_mass == 0.0_r;
}

void phys::rigid_body::setup() {
	force = vec3(0.0_r);
	torque = vec3(0.0_r);
}

void phys::rigid_body::add_force(const vec3 &f) {
	force += f;
}

void phys::rigid_body::add_force_at_world(const vec3 &f_world, const vec3 &at_world) {
	vec3 d = at_world - pos;
	vec3 t = cross(d, f_world);

	force += f_world;
	torque += t;
}

void phys::rigid_body::add_force_at_local(const vec3 &f_world, const vec3 &at_local) {
	add_force_at_world(f_world, truncate(local_to_world * vec4(at_local, 1.0_r)));
}

void phys::rigid_body::calculate_derived_data() {
	calculate_local_to_world();
	calculate_inv_inertia_tensor_world();
}

void phys::rigid_body::calculate_local_to_world() {
	local_to_world[0] = rot * vec4(1.0_r, 0.0_r, 0.0_r, 0.0_r);
	local_to_world[1] = rot * vec4(0.0_r, 1.0_r, 0.0_r, 0.0_r);
	local_to_world[2] = rot * vec4(0.0_r, 0.0_r, 1.0_r, 0.0_r);
	local_to_world[3] = vec4(pos, 1.0_r);
}

void phys::rigid_body::calculate_inv_inertia_tensor_world() {
	mat3 local_to_world_rot = truncate(local_to_world);

	inv_inertia_tensor_world = local_to_world_rot * inv_inertia_tensor * inverse(local_to_world_rot);
}

void phys::rigid_body::integrate(real dt) {
	vec3 prev_acc = acc;
	prev_acc += force * inv_mass;

	vec3 ang_acc = inv_inertia_tensor_world * torque;
	vel += prev_acc * dt;
	ang_vel += ang_acc * dt;

	vel *= std::pow(linear_damping, dt);
	ang_vel *= std::pow(angular_damping, dt);

	calculate_derived_data();
	setup();
}

const phys::mat4& phys::rigid_body::get_transform() const {
	return local_to_world;
}
