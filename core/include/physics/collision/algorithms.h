#pragma once
#include "algorithm.h"

namespace phys::algorithms {
	extern collision_algorithm_func sphere_sphere_collision;
	extern collision_algorithm_func sphere_plane_collision;
	extern collision_algorithm_func plane_plane_collision;

	void init_algorithms();
}