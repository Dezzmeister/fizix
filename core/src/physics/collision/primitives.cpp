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
	const vec3 &_normal,
	real _offset
) :
	primitive(shape_type::Plane, _body, identity<mat4>()),
	normal(_normal),
	offset(_offset)
{}

phys::box::box(
	rigid_body * _body,
	const mat4 &_offset,
	const vec3 &_half_size
) :
	primitive(shape_type::Box, _body, _offset),
	half_size(_half_size)
{}