#pragma once
#include "rigid_body_force_generator.h"

namespace phys {
	class rigid_body_gravity : public rigid_body_force_generator {
	public:
		vec3 gravity;

		rigid_body_gravity(const vec3 &_gravity);

		void update_force(rigid_body &body, real dt);
	};
}
