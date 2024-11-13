#pragma once
#include <string>
#include <variant>
#include "geometry.h"

namespace shapes {
	extern std::unique_ptr<geometry> cube;
	extern std::unique_ptr<geometry> plane;

	// This must be called once before using any of the predefined shapes, or the shape creation
	// functions.
	void init();

	geometry make_sphere(size_t horizontal_divisions, size_t vertical_divisions, bool smooth_normals);
	geometry make_cylinder(size_t divisions, bool smooth_normals);
};
