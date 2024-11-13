#include "physics/collision/algorithms.h"
#include "physics/collision/primitives.h"

using namespace phys::literals;

phys::collision_algorithm_func phys::algorithms::sphere_sphere_collision =
[](primitive &_a, primitive &_b, contact_container &contacts) {
	sphere &a = static_cast<sphere&>(_a);
	sphere &b = static_cast<sphere&>(_b);

	vec3 pos_a = a.body->pos + truncate(a.offset[3]);
	vec3 pos_b = b.body->pos + truncate(b.offset[3]);
	vec3 d_vec = pos_b - pos_a;
	real d = std::sqrt(dot(d_vec, d_vec));
	real min_radius = a.radius + b.radius;

	if (d >= min_radius) {
		return;
	}

	contact c(
		a.body,
		b.body,
		pos_a + d_vec / 2.0_r,
		d_vec / d,
		min_radius - d
	);

	contacts.insert(std::end(contacts), c);
	};