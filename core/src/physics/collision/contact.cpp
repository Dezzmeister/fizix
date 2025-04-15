#include <iostream>
#include <tuple>
#include "physics/collision/contact.h"
#include "util.h"

using namespace phys::literals;

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

phys::contact phys::contact::from_vclip(
	rigid_body *_a,
	rigid_body *_b,
	vclip::algorithm_result &_result
) {
	vec3 point{};
	vec3 normal{};
	real penetration{};

	if (
		std::holds_alternative<vclip::vertex>(_result.state.f1) &&
		std::holds_alternative<vclip::face>(_result.state.f2)
	) {
		point = std::get<vclip::vertex>(_result.state.f1).v;
		normal = std::get<vclip::face>(_result.state.f2).normal(_result.p2);
		penetration = -_result.state.penetration;
	} else if (
		std::holds_alternative<vclip::face>(_result.state.f1) &&
		std::holds_alternative<vclip::vertex>(_result.state.f2)
	) {
		point = std::get<vclip::vertex>(_result.state.f2).v;
		normal = std::get<vclip::face>(_result.state.f1).normal(_result.p1);
		penetration = -_result.state.penetration;
	} else if (
		std::holds_alternative<vclip::edge>(_result.state.f1) &&
		std::holds_alternative<vclip::face>(_result.state.f2)
	) {
		const vclip::edge &e = std::get<vclip::edge>(_result.state.f1);
		const vclip::face &f = std::get<vclip::face>(_result.state.f2);
		// TODO: This is a hack! Don't do this
		point = (e.h(_result.p1).v + e.t(_result.p1).v) / 2.0_r;
		normal = f.normal(_result.p2);
		penetration = -_result.state.penetration;
	} else if (
		std::holds_alternative<vclip::face>(_result.state.f1) &&
		std::holds_alternative<vclip::edge>(_result.state.f2)
	) {
		const vclip::face &f = std::get<vclip::face>(_result.state.f1);
		const vclip::edge &e = std::get<vclip::edge>(_result.state.f2);
		// TOOD: Hack
		point = (e.h(_result.p2).v + e.t(_result.p2).v) / 2.0_r;
		normal = f.normal(_result.p1);
		penetration = -_result.state.penetration;
	} else {
		// TODO: Proper error
		throw "Unexpected vclip::algorithm_result";
	}

	return contact(
		_a,
		_b,
		point,
		normal,
		penetration
	);
}

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