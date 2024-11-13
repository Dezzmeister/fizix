#include "physics/collision/primitives.h"

phys::sphere::sphere(
	rigid_body * _body,
	const mat4 &_offset,
	real _radius
) :
	primitive(shape_type::Sphere, _body, _offset),
	radius(_radius)
{}