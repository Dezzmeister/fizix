#include "physics/collision/primitives.h"

phys::sphere::sphere(
	rigid_body * _body,
	const mat4 &_offset,
	real _radius
) :
	primitive(shape_type::Sphere, _body, _offset),
	radius(_radius)
{}

phys::plane::plane(
	rigid_body * _body,
	const vec3 &_dir
) :
	primitive(shape_type::Plane, _body, identity<mat4>()),
	dir(_dir)
{}