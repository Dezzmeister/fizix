#pragma once
#include <glm/glm.hpp>
#include <memory>
#include "geometry.h"
#include "phong_color_material.h"
#include "physics/math.h"

extern const phong_color_material sphere_mtl;
extern const phong_color_material selected_sphere_mtl;

extern const phys::real sphere_radius;
extern const glm::mat4 sphere_scale;
extern std::unique_ptr<geometry> sphere_geom;

extern void init_constants();
