#include "physics/particle_force_generators.h"

phys::particle_gravity::particle_gravity(const vec3 &_gravity) :
	particle_force_generator(),
	gravity(_gravity)
{}

void phys::particle_gravity::update_force(particle &p, real) {
	if (! p.has_finite_mass()) {
		return;
	}

	p.force += gravity * p.get_mass();
}