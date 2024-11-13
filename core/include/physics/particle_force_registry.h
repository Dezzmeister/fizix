#pragma once
#include <vector>
#include "math.h"
#include "particle.h"
#include "particle_force_generator.h"

namespace phys {
	struct particle_force_registration {
		particle * p;
		particle_force_generator * fg;

		friend bool operator==(const particle_force_registration &a, const particle_force_registration &b);
	};

	class particle_force_registry {
	public:
		void add(particle * p, particle_force_generator * fg);
		void remove(particle * p, particle_force_generator * fg);
		void clear();
		void update_forces(real duration);

	private:
		using registry = std::vector<particle_force_registration>;

		registry registrations{};
	};
}
