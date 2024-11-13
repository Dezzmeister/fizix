#include <algorithm>
#include <ranges>
#include "physics/particle_world.h"

using namespace phys::literals;

phys::particle_world::particle_world(
	uint64_t _solver_iterations,
	real _min_pos_change
) :
	solver_iterations(_solver_iterations),
	inv_solver_iterations(1.0_r / (real)_solver_iterations),
	min_pos_change_sqr(_min_pos_change * _min_pos_change)
{}

void phys::particle_world::prepare_frame() {
	for (particle * p : particles) {
		p->force = phys::vec3(0.0_r);
		p->acc = phys::vec3(0.0_r);
	}

	collision_constraints.clear();
}

void phys::particle_world::run_physics(real dt) {
	if (dt == 0.0_r) {
		return;
	}

	force_registry.update_forces(dt);

	for (particle * p : particles) {
		p->vel += (dt * p->get_inv_mass() * p->force * p->damping);
	}

	for (particle * p : particles) {
		p->p = p->pos + dt * p->vel;
	}

	generate_collision_constraints(dt);
	solve_constraints(dt);

	for (particle * p : particles) {
		const vec3 dp = p->pos - p->p;

		if (phys::dot(dp, dp) <= min_pos_change_sqr) {
			p->p = p->pos;
		}

		p->vel = (p->p - p->pos) / dt;
		p->pos = p->p;
	}

	for (constraint * c : fixed_constraints) {
		c->update_velocities(dt);
	}

	for (std::unique_ptr<constraint> &c : collision_constraints) {
		c->update_velocities(dt);
	}
}

void phys::particle_world::generate_collision_constraints(real dt) {
	for (constraint_generator * cg : constraint_generators) {
		cg->generate_constraints(dt, collision_constraints);
	}
}

void phys::particle_world::solve_constraints(real) {
	for (size_t i = 0; i < solver_iterations; i++) {
		size_t num_projected = 0;

		if (solve_forward) {
			for (constraint * c : fixed_constraints) {
				if (! c->is_satisfied()) {
					c->project(inv_solver_iterations);
					num_projected++;
				}
			}

			for (std::unique_ptr<constraint> &c : collision_constraints) {
				if (! c->is_satisfied()) {
					c->project(inv_solver_iterations);
					num_projected++;
				}
			}
		} else {
			for (std::unique_ptr<constraint> &c : std::ranges::reverse_view(collision_constraints)) {
				if (! c->is_satisfied()) {
					c->project(inv_solver_iterations);
					num_projected++;
				}
			}

			for (constraint * c : std::ranges::reverse_view(fixed_constraints)) {
				if (! c->is_satisfied()) {
					c->project(inv_solver_iterations);
					num_projected++;
				}
			}
		}

		solve_forward = ! solve_forward;

		if (! num_projected) {
			return;
		}
	}
}

void phys::particle_world::add_particle(particle * p) {
	particles.push_back(p);
}

void phys::particle_world::remove_particle(particle *p) {
	std::erase(particles, p);
}

void phys::particle_world::add_constraint_generator(constraint_generator * generator) {
	constraint_generators.push_back(generator);
}

void phys::particle_world::remove_constraint_generator(constraint_generator * generator) {
	std::erase(constraint_generators, generator);
}

void phys::particle_world::add_fixed_constraint(constraint * c) {
	fixed_constraints.push_back(c);
}

void phys::particle_world::remove_fixed_constraint(constraint * c) {
	std::erase(fixed_constraints, c);
}