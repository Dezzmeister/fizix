#include "raycast.h"

using namespace phys;
using namespace phys::literals;

// This works by solving a system of equations for a single parameter, `t`.
// The equation of the line is `x = p + t * d`, and the equation of the sphere
// is `(x - s) * (x - s) = r * r`. (Note that `x` is a vector in both equations.)
// The line equation can be substituted into the sphere equation, which forms an
// equation quadratic in `t`. To constrain `t` to solutions on the ray (and not the
// line), we need to solve the quadratic equation and discard negative solutions.
real raycast_sphere_test(const vec3 &p, const vec3 &d, const vec3 &s, real r) {
	vec3 n = p - s;
	real c = dot(n, n) - r * r;
	real b = 2 * dot(n, d);
	real a = dot(d, d);

	real discrim = b * b - (4 * a * c);

	if (discrim < 0) {
		return -1.0_r;
	}

	real t0 = (-b + discrim) / (2 * a);
	real t1 = (-b - discrim) / (2 * a);

	real min_t = std::min(t0, t1);

	if (min_t < 0) {
		real max_t = std::max(t0, t1);

		if (max_t < 0) {
			// This can happen if we're in front of the sphere
			return -1.0_r;
		}

		// This would mean we're inside the sphere
		return max_t;
	}

	return min_t;
}