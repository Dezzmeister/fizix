#include "shapes.h"
#include "constants.h"

// Ruby
const phong_color_material sphere_mtl{
	phong_color_material_properties{
		glm::vec3(0.1745, 0.01175, 0.01175),
		glm::vec3(0.61424, 0.04136, 0.04136),
		glm::vec3(0.727811, 0.626959, 0.626959),
		128 * 0.6f
	}
};

// Gold
const phong_color_material selected_sphere_mtl{
	phong_color_material_properties{
		glm::vec3(0.24725, 0.1995, 0.0745),
		glm::vec3(0.75164, 0.60648, 0.22648),
		glm::vec3(0.628281, 0.555802, 0.366065),
		128 * 0.4f
	}
};

const float sphere_radius = 0.1f;
const glm::mat4 sphere_scale = glm::scale(glm::identity<glm::mat4>(),
	glm::vec3(sphere_radius / 0.5f, sphere_radius / 0.5f, sphere_radius / 0.5f));
std::unique_ptr<geometry> sphere_geom{};

void init_constants() {
	sphere_geom = std::make_unique<geometry>(shapes::make_sphere(20, 10, true));
}