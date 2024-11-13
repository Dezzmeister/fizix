#include <cmath>
#include <stdexcept>
#include "physics/particle.h"

using namespace phys::literals;

phys::particle::particle() :
	damping(0.995_r),
	radius(0.0_r),
	inv_mass(1.0_r)
{}

void phys::particle::integrate(real dt) {
	pos += (vel * dt);
	acc += force * inv_mass;
	vel += (acc * dt);
	vel *= pow(damping, dt);
}

void phys::particle::set_mass(real mass) {
	if (mass == infinity) {
		inv_mass = 0.0_r;
		// TODO: epsilon
	} else if (mass == 0.0_r) {
		throw std::invalid_argument("Mass must be nonzero");
	} else {
		inv_mass = 1.0_r / mass;
	}
}

phys::real phys::particle::get_mass() const {
	// TODO: epsilon
	if (inv_mass == 0.0_r) {
		return infinity;
	}

	return 1.0_r / inv_mass;
}

phys::real phys::particle::get_inv_mass() const {
	return inv_mass;
}

bool phys::particle::has_finite_mass() const {
	// TODO: epsilon
	return inv_mass != 0.0_r;
}