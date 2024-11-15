#include "physics/collision/algorithms.h"
#include "physics/collision/primitives.h"

using namespace phys::literals;

namespace {
	bool initialized{};
}

phys::collision_algorithm_func phys::algorithms::sphere_sphere_collision;
phys::collision_algorithm_func phys::algorithms::sphere_plane_collision;
phys::collision_algorithm_func phys::algorithms::plane_plane_collision;

void phys::algorithms::init_algorithms() {
	if (initialized) {
		return;
	}

	initialized = true;

	sphere_sphere_collision =
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

	sphere_plane_collision =
		[](primitive &_a, primitive &_b, contact_container &contacts) {
			sphere &s = static_cast<sphere&>(_a);
			plane &p = static_cast<plane&>(_b);

			vec3 s_pos = s.body->pos + truncate(s.offset[3]);
			// TODO: Memoize
			real p_len = std::sqrt(dot(p.dir, p.dir));
			real overlap = dot(s_pos, p.dir) / p_len;
			real diff = std::abs(overlap - p_len);

			if (s.radius < diff) {
				return;
			}

			vec3 contact_norm = normalize(p.dir);
			vec3 contact_pt = s_pos + contact_norm * diff;

			contact c(
				s.body,
				p.body,
				contact_pt,
				contact_norm,
				s.radius - diff
			);

			contacts.insert(std::end(contacts), c);
		};

	plane_plane_collision =
		[](primitive&, primitive&, contact_container&) {
			// Infinite planes are always in contact unless they're perfectly
			// parallel, so this type of collision probably isn't very useful
			// and we can ignore it
		};
}