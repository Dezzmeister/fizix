#include "physics/constraints.h"

using namespace phys::literals;

phys::particle_collision_constraint::particle_collision_constraint(
	particle * _a,
	particle * _b,
	real _restitution,
	real _friction
) :
	particle_constraint<2>(1.0_r, phys::constraint_type::Inequality, { _a, _b }),
	restitution(_restitution),
	friction(_friction),
	a_old_vel(_a->vel),
	b_old_vel(_b->vel),
	a_old_pos(_a->p),
	b_old_pos(_b->p)
{}

phys::real phys::particle_collision_constraint::eval_constraint() const {
	vec3 diff = a()->p - b()->p;

	return std::sqrt(phys::dot(diff, diff)) - (a()->radius + b()->radius);
}

phys::vec3 phys::particle_collision_constraint::eval_gradient(const particle &p) const {
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

void phys::particle_collision_constraint::update_velocities(real dt) {
	if (a_old_pos == b_old_pos) {
		return;
	}

	real ma = a()->get_mass();
	real mb = b()->get_mass();
	real m_total = ma + mb;
	real e = restitution;

	if (m_total == 0.0_r || (! a()->has_finite_mass() && ! b()->has_finite_mass())) {
		return;
	}

	vec3 n = phys::normalize(b_old_pos - a_old_pos);
	decomposed_vec3 a_old_parts = decompose_vec3(a_old_vel, n);
	decomposed_vec3 b_old_parts = decompose_vec3(b_old_vel, n);
	decomposed_vec3 a_new_parts = decompose_vec3(a()->vel, n);
	decomposed_vec3 b_new_parts = decompose_vec3(b()->vel, n);

	vec3 va{};
	vec3 vb{};

	if (! a()->has_finite_mass()) {
		vb = -e * b_old_parts.parallel + (1 + e) * a_old_parts.parallel;
	} else if (! b()->has_finite_mass()) {
		va = -e * a_old_parts.parallel + (1 + e) * b_old_parts.parallel;
	} else {
		va = ((ma - e * mb) / m_total) * a_old_parts.parallel + ((mb + e * mb) / m_total) * b_old_parts.parallel;
		vb = ((mb - e * ma) / m_total) * b_old_parts.parallel + ((ma + e * ma) / m_total) * a_old_parts.parallel;
	}

	real f = 1 - friction * dt;

	a()->vel = va + f * a_new_parts.perp;
	b()->vel = vb + f * b_new_parts.perp;
}