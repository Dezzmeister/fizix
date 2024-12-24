#pragma once
#include <optional>
#include "math.h"

namespace phys {
	// Finds intersection parameters for two line segments. Assumes the
	// line segments are coplanar. The lines are represented by a starting
	// point 'v' and a segment vector 'd', so that a line segment starts at v
	// and ends at v + d. Points along the line are parameterized by 't' like:
	// p = v + t * d
	// The result is a pair of intersection parameters t1 and t2 for two line
	// segments:
	// p1 = v1 + t1 * d1
	// p2 = v2 + t2 * d2
	// The returned parameters solve the system of equations given by:
	// v1 + t1 * d1 = v2 + t2 * d2
	// The result is empty iff the line segments are parallel.
	// The line segments intersect iff 0 <= t1 <= 1 and 0 <= t2 <= 1.
	std::optional<vec2> line_line_intersection(
		// Starting point for the first line
		const vec3 &v1,
		// Segment vector for the first line
		const vec3 &d1,
		// Starting point for the second line
		const vec3 &v2,
		// Segment vector for the second line
		const vec3 &d2
	);
}