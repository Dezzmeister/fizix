#pragma once
#include "physics/math.h"

// Determines if a ray intersects a sphere. The return value is a non-negative parameter
// if the ray intersects the sphere, and -1.0 if not. The parameter can be used to calculate
// the intersection point: if `t` is the parameter, the intersection point is `pos + t * dir`.
// If there is more than one intersection point, the closest one is returned.
//
// The ray starts at `pos` and continues in the direction of `dir`. `sphere_pos` is the center of
// the sphere, and `r` is the radius of the sphere.
phys::real raycast_sphere_test(const phys::vec3 &pos, const phys::vec3 &dir, const phys::vec3 &sphere_pos, phys::real r);
