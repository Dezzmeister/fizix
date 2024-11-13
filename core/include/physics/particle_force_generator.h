#pragma once
#include "math.h"
#include "particle.h"

namespace phys {
	class particle_force_generator {
	public:
		virtual ~particle_force_generator() = default;

		virtual void update_force(particle &p, real duration) = 0;
	};
}
