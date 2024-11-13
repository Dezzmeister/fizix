#pragma once
#include <memory>
#include <vector>
#include "constraint.h"
#include "particle.h"
#include "particle_force_registry.h"

namespace phys {
	class particle_world {
	public:
		particle_force_registry force_registry{};

		particle_world(
			uint64_t _solver_iterations,
			real _min_pos_change = (real)0.00005
		);

		void prepare_frame();
		void run_physics(real dt);

		void add_particle(particle * p);
		void remove_particle(particle * p);

		void add_fixed_constraint(constraint * c);
		void remove_fixed_constraint(constraint * c);

		void add_constraint_generator(constraint_generator * generator);
		void remove_constraint_generator(constraint_generator * generator);

	private:
		std::vector<particle *> particles{};
		std::vector<constraint_generator *> constraint_generators{};
		std::vector<constraint *> fixed_constraints{};
		std::vector<std::unique_ptr<constraint>> collision_constraints{};
		size_t solver_iterations;
		real inv_solver_iterations;
		real min_pos_change_sqr;
		bool solve_forward{};

		void generate_collision_constraints(real dt);
		void solve_constraints(real dt);
	};
}
