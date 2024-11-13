#pragma once
#include "rigid_body.h"

namespace phys {
	class rigid_body_force_generator {
	public:
		virtual ~rigid_body_force_generator() = default;

		virtual void update_force(rigid_body &body, real dt) = 0;
	};
}
