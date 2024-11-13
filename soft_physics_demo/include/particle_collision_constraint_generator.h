#pragma once
#include <array>
#include <bitset>
#include "physics/constraints.h"
#include "physics/particle.h"

template <const size_t N>
class particle_collision_constraint_generator : public phys::constraint_generator {
public:
	std::array<phys::particle, N> &particles;
	std::bitset<N> &active;

	particle_collision_constraint_generator(
		std::array<phys::particle, N> &_particles,
		std::bitset<N> &_active
	);

	void generate_constraints(phys::real dt, std::vector<std::unique_ptr<phys::constraint>> &constraints) override;
};

template <const size_t N>
particle_collision_constraint_generator<N>::particle_collision_constraint_generator(
	std::array<phys::particle, N> &_particles,
	std::bitset<N> &_active
) :
	constraint_generator(),
	particles(_particles),
	active(_active)
{}

template <const size_t N>
void particle_collision_constraint_generator<N>::generate_constraints(
	phys::real,
	std::vector<std::unique_ptr<phys::constraint>> &constraints
) {
	using namespace phys::literals;

	std::vector<size_t> active_particles{};

	for (size_t i = 0; i < N; i++) {
		if (active[i]) {
			active_particles.push_back(i);
		}
	}

	for (size_t i = 0; i < active_particles.size(); i++) {
		for (size_t j = i + 1; j < active_particles.size(); j++) {
			phys::particle &a = particles[active_particles[i]];
			phys::particle &b = particles[active_particles[j]];

			phys::particle_collision_constraint c(&a, &b, 0.9_r, 0.1_r);

			if (c.eval_constraint() < 0.0_r) {
				constraints.push_back(std::make_unique<phys::particle_collision_constraint>(c));
			}
		}
	}
}
