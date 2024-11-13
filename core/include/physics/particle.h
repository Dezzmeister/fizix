#pragma once
#include "math.h"

namespace phys {
	class particle {
	public:
		vec3 pos{};
		vec3 vel{};
		vec3 acc{};
		vec3 force{};
		// Position estimate
		vec3 p{};
		vec3 p_accum{};
		size_t n{};

		real damping;
		// TODO: Move this out into the spherical particle contact generator.
		// Particles in general won't have a radius
		real radius;

		particle();

		void integrate(real dt);
		void set_mass(real mass);
		real get_mass() const;
		real get_inv_mass() const;
		bool has_finite_mass() const;

	private:
		real inv_mass;
	};
}
