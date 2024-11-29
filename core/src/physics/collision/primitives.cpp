#include "physics/collision/primitives.h"

using namespace phys::literals;

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

phys::vclip::polyhedron phys::box::to_polyhedron() const {
	mat4 transform = body->get_transform() * offset;

	vclip::polyhedron p(
		{
			vclip::vertex(truncate(transform * vec4(half_size, 1.0_r)), 0),
			vclip::vertex(truncate(transform * vec4(half_size.x, half_size.y, -half_size.z, 1.0_r)), 1),
			vclip::vertex(truncate(transform * vec4(half_size.x, -half_size.y, half_size.z, 1.0_r)), 2),
			vclip::vertex(truncate(transform * vec4(half_size.x, -half_size.y, -half_size.z, 1.0_r)), 3),
			vclip::vertex(truncate(transform * vec4(-half_size.x, half_size.y, half_size.z, 1.0_r)), 4),
			vclip::vertex(truncate(transform * vec4(-half_size.x, half_size.y, -half_size.z, 1.0_r)), 5),
			vclip::vertex(truncate(transform * vec4(-half_size.x, -half_size.y, half_size.z, 1.0_r)), 6),
			vclip::vertex(truncate(transform * vec4(-half_size.x, -half_size.y, -half_size.z, 1.0_r)), 7)
		},
		{
			vclip::edge(0, 1),
			vclip::edge(0, 2),
			vclip::edge(0, 4),
			vclip::edge(1, 3),
			vclip::edge(1, 5),
			vclip::edge(2, 3),
			vclip::edge(2, 6),
			vclip::edge(3, 7),
			vclip::edge(4, 5),
			vclip::edge(4, 6),
			vclip::edge(5, 7),
			vclip::edge(6, 7)
		},
		{
			vclip::face({ 0, 1, 5, 4 }),
			vclip::face({ 0, 2, 3, 1 }),
			vclip::face({ 0, 4, 6, 2 }),
			vclip::face({ 7, 3, 2, 6 }),
			vclip::face({ 7, 6, 4, 5 }),
			vclip::face({ 7, 5, 1, 3 })
		}
	);

	return p;
}