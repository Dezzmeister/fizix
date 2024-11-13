#include "physics/constraints.h"

using namespace phys::literals;

phys::distance_constraint::distance_constraint(
	particle * _a,
	particle * _b,
	real _distance,
	real _stiffness
) :
	particle_constraint<2>(_stiffness, constraint_type::Equality, { _a, _b }),
	distance(_distance)
{}

phys::real phys::distance_constraint::eval_constraint() const {
	vec3 dx = a()->p - b()->p;

	return std::sqrt(phys::dot(dx, dx)) - distance;
}

phys::vec3 phys::distance_constraint::eval_gradient(const particle &p) const {
	if (a()->p == b()->p) {
		return vec3(0.0_r);
	}

	vec3 n = phys::normalize(a()->p - b()->p);

	if (&p == a()) {
		return n;
	} else if (&p == b()) {
		return -n;
	}

	return vec3(0.0_r);
}

void phys::distance_constraint::update_velocities(real) {
	// TODO
}
