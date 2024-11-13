#include "physics/rigid_body_force_generators.h"

phys::rigid_body_gravity::rigid_body_gravity(const vec3 &_gravity) :
	gravity(_gravity)
{}

void phys::rigid_body_gravity::update_force(rigid_body &body, real) {
	if (! body.has_finite_mass()) {
		return;
	}

	body.add_force(gravity * body.get_mass());
}