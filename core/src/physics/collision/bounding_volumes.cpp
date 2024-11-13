#define _USE_MATH_DEFINES
#include <cmath>
#include <tuple>
#include "physics/collision/bounding_volumes.h"

using namespace phys::literals;

phys::bounding_sphere::bounding_sphere(const vec3 &_center, real _radius) :
	center(_center),
	radius(_radius)
{}

phys::bounding_sphere::bounding_sphere(const bounding_sphere &a, const bounding_sphere &b) {
	vec3 d_vec = b.center - a.center;
	real d_len = std::sqrt(dot(d_vec, d_vec));

	real c = ((d_len + a.radius + b.radius) / 2.0_r) - a.radius;

	d_vec /= d_len;

	center = a.center + d_vec * c;
	radius = (d_len + a.radius + b.radius) / 2.0_r;
}

bool phys::bounding_sphere::overlaps(const bounding_sphere &other) const {
	vec3 d_vec = other.center - center;
	real dsqr = dot(d_vec, d_vec);

	return dsqr <= (other.radius + radius) * (other.radius + radius);
}

phys::real phys::bounding_sphere::growth(const bounding_sphere &other) const {
	const bounding_sphere overlap(*this, other);

	return overlap.volume() - volume();
}

phys::real phys::bounding_sphere::volume() const {
	return 4.0_r * (real)M_PI * radius * radius * radius / 3.0_r;
}

bool phys::operator==(const bounding_sphere &a, const bounding_sphere &b) {
	return std::tie(a.center, a.radius) == std::tie(b.center, b.radius);
}