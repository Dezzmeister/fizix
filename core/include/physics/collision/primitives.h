#pragma once
#include "primitive.h"

namespace phys {
	class sphere : public primitive {
	public:
		real radius;

		sphere(rigid_body * _body, const mat4 &_offset, real _radius);
	};

	class plane : public primitive {
	public:
		// The direction of the plane. The length of this vector gives the
		// "position" of the plane along the direction.
		vec3 dir;

		plane(rigid_body * _body, const vec3 &_dir);
	};
}
