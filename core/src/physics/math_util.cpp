#include "physics/math_util.h"

using namespace phys::literals;

std::optional<phys::vec2> phys::line_line_intersection(
	const vec3 &v1,
	const vec3 &d1,
	const vec3 &v2,
	const vec3 &d2
) {
	// We need to solve for t where A = [ d1 d2 ]:
	// A [ t ] = [ v2 - v1 ]
	// A is a 3x2 matrix, so it doesn't have a proper inverse. We can
	// still find a "left inverse" and multiply by [ v2 - v1 ], but we
	// need to make sure that we pick linearly independent rows with which
	// to find the inverse
	// TODO?: handle colinear line segments?
	vec3 dps[3] = {
		vec3(d1.x, -d2.x, v2.x - v1.x),
		vec3(d1.y, -d2.y, v2.y - v1.y),
		vec3(d1.z, -d2.z, v2.z - v1.z)
	};

	vec3 dp1 = dps[0];
	vec3 dp2 = dps[1];

	if (dp1.x == 0.0_r && dp1.y == 0.0_r) {
		// The first row is zero
		dp1 = dps[2];
	} else if (dp2.x == 0.0_r && dp2.y == 0.0_r) {
		// The second row is zero
		dp2 = dps[2];
	} else if (dp1.x == dp2.x && dp1.y == dp2.y) {
		// The first two rows are equal
		dp2 = dps[2];
	}

	real det = (dp1.x * dp2.y) - (dp1.y * dp2.x);

	if (det == 0.0_r) {
		return std::nullopt;
	}

	real inv_det = 1.0_r / det;
	real t1 = inv_det * (dp2.y * dp1.z + -dp1.y * dp2.z);
	real t2 = inv_det * (-dp2.x * dp1.z + dp1.x * dp2.z);

	return vec2(t1, t2);
}