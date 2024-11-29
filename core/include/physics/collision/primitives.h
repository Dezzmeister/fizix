#pragma once
#include "primitive.h"
#include "vclip.h"

namespace phys {
	class sphere : public primitive {
	public:
		real radius;

		sphere(rigid_body * _body, const mat4 &_offset, real _radius);
	};

	class plane : public primitive {
	public:
		vec3 normal;
		real offset;

		plane(rigid_body * _body, const vec3 &_normal, real _offset);
	};

	class box : public primitive {
	public:
		vec3 half_size;

		box(rigid_body * _body, const mat4 &_offset, const vec3 &_half_size);

		// Converts the box to a VClip polyhedron. The polyhedron is expressed in
		// world space.
		vclip::polyhedron to_polyhedron() const;
	};
}
