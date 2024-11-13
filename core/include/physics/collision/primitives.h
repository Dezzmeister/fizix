#pragma once
#include "primitive.h"

namespace phys {
	class sphere : public primitive {
	public:
		real radius;

		sphere(rigid_body * _body, const mat4 &_offset, real _radius);
	};
}
