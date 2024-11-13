#include <iostream>
#include <tuple>
#include "physics/collision/contact.h"
#include "util.h"

phys::contact::contact(
	rigid_body * _a,
	rigid_body * _b,
	const vec3 &_point,
	const vec3 &_normal,
	real _penetration
) :
	a(_a),
	b(_b),
	point(_point),
	normal(_normal),
	penetration(_penetration)
{}

bool phys::operator==(const contact &a, const contact &b) {
	return std::tie(a.a, a.b) == std::tie(b.a, b.b) &&
		util::eq_within_epsilon(a.point, b.point) &&
		util::eq_within_epsilon(a.normal, b.normal) &&
		util::eq_within_epsilon(a.penetration, b.penetration);
}

template <>
std::string traits::to_string(const phys::contact &c, size_t indent) {
	using namespace util;

	return obj_to_string(
		"contact",
		indent,
		named_val("a", c.a),
		named_val("b", c.b),
		named_val("point", c.point),
		named_val("normal", c.normal),
		named_val("penetration", c.penetration)
	);
}