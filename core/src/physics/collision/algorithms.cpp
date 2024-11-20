#include <algorithm>
#include <array>
#include "physics/collision/algorithms.h"
#include "physics/collision/primitives.h"

using namespace phys::literals;

namespace {
	bool initialized{};

	phys::real dist_from_plane(const phys::vec3 &p, const phys::vec3 &normal, phys::real offset) {
		return offset - phys::dot(p, normal);
	}

	struct pos_and_dist {
		phys::vec3 pos{};
		phys::real dist{};

		void set_pos(const phys::vec3 &_pos, const phys::vec3 &dir, phys::real dir_len) {
			pos = _pos;
			dist = dist_from_plane(pos, dir, dir_len);
		}
	};
}

phys::collision_algorithm_func phys::algorithms::sphere_sphere_collision;
phys::collision_algorithm_func phys::algorithms::sphere_plane_collision;
phys::collision_algorithm_func phys::algorithms::plane_plane_collision;
phys::collision_algorithm_func phys::algorithms::plane_box_collision;
phys::collision_algorithm_func phys::algorithms::sphere_box_collision;
phys::collision_algorithm_func phys::algorithms::box_box_collision;

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
			real overlap = dot(s_pos, p.normal);
			real diff = std::abs(overlap - p.offset);

			if (s.radius < diff) {
				return;
			}

			vec3 contact_pt = s_pos + p.normal * diff;

			contact c(
				s.body,
				p.body,
				contact_pt,
				p.normal,
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

	plane_box_collision =
		[](primitive &a, primitive &_b, contact_container &contacts) {
			static std::array<pos_and_dist, 8> plane_dists{};

			plane &p = static_cast<plane&>(a);
			box &b = static_cast<box&>(_b);

			// We can transform b.body->pos directly here because the center of mass
			// is not affected by local rotation
			vec3 b_pos = vec3(b.offset * vec4(b.body->pos, 1.0_r));
			// The sign of `center_diff` tells us which side of the plane
			// the box is on
			real center_diff = dist_from_plane(b_pos, p.normal, p.offset);
			bool center_diff_side = center_diff < 0;

			plane_dists[0].pos = b.half_size;
			plane_dists[1].pos = vec3(b.half_size.x, b.half_size.y, -b.half_size.z);
			plane_dists[2].pos = vec3(b.half_size.x, -b.half_size.y, b.half_size.z);
			plane_dists[3].pos = vec3(b.half_size.x, -b.half_size.y, -b.half_size.z);
			plane_dists[4].pos = vec3(-b.half_size.x, b.half_size.y, b.half_size.z);
			plane_dists[5].pos = vec3(-b.half_size.x, b.half_size.y, -b.half_size.z);
			plane_dists[6].pos = vec3(-b.half_size.x, -b.half_size.y, b.half_size.z);
			plane_dists[7].pos = -b.half_size;

			mat4 transform = b.body->get_transform() * b.offset;

			for (size_t i = 0; i < 8; i++) {
				plane_dists[i].pos = vec3(transform * vec4(plane_dists[i].pos, 1.0_r));
				plane_dists[i].dist = dist_from_plane(plane_dists[i].pos, p.normal, p.offset);
			}

			auto partition_it = std::partition(std::begin(plane_dists), std::end(plane_dists),
				[=](const pos_and_dist &a) {
					return (a.dist < 0) == center_diff_side;
				});

			if (partition_it == std::begin(plane_dists) || partition_it == std::end(plane_dists)) {
				// All of the box's vertices are on one side of the plane
				return;
			}

			// Only those vertices which are on the other side of the plane are
			// in contact with the plane
			for (; partition_it != std::end(plane_dists); partition_it++) {
				contact c(
					b.body,
					p.body,
					partition_it->pos,
					center_diff_side ? -p.normal : p.normal,
					std::abs(partition_it->dist)
				);

				contacts.insert(std::end(contacts), c);
			}
		};

	sphere_box_collision =
		[](primitive &a, primitive &_b, contact_container &contacts) {
			sphere &s = static_cast<sphere&>(a);
			box &b = static_cast<box&>(_b);

			vec3 s_pos_w = s.body->pos + truncate(s.offset[3]);
			// Sphere pos in the box frame
			vec3 s_pos_b = truncate(
				b.get_inv_offset() * b.body->get_inv_transform() * vec4(s_pos_w, 1.0_r)
			);

			// In the box frame, the box is aligned with the Cartesian axes, and the box
			// is at the origin

			// TODO: Transform the radius as a direction to account for scaling
			// transforms
			if (
				std::abs(s_pos_b.x) - s.radius > b.half_size.x ||
				std::abs(s_pos_b.y) - s.radius > b.half_size.y ||
				std::abs(s_pos_b.z) - s.radius > b.half_size.z
			) {
				return;
			}

			vec3 closest_pt_b = clamp(s_pos_b, -b.half_size, b.half_size);
			vec3 diff_b = s_pos_b - closest_pt_b;
			real dist_sqr_b = dot(diff_b, diff_b);

			if (dist_sqr_b >= s.radius * s.radius) {
				return;
			}

			vec3 closest_pt_w = truncate(b.body->get_transform() * b.offset * vec4(closest_pt_b, 1.0_r));
			vec3 diff_w = closest_pt_w - s_pos_w;

			contact c(
				s.body,
				b.body,
				closest_pt_w,
				normalize(diff_w),
				s.radius - std::sqrt(dot(diff_w, diff_w))
			);

			contacts.insert(std::end(contacts), c);
		};

	box_box_collision =
		[](primitive&, primitive&, contact_container&) {
			// TODO: Implement this
		};
}