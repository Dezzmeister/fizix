#include "constants.h"
#include "particle_utils.h"

void move_mesh_to_particle(mesh * m, const phys::particle * p) {
	m->set_model(glm::translate(
		glm::identity<glm::mat4>(),
		phys::to_glm<glm::vec3>(p->pos)
	) * sphere_scale);
}

void hide_particle(instanced_mesh * particles, size_t i) {
	particles->set_model(i, glm::translate(
		glm::identity<glm::mat4>(),
		glm::vec3(0.0f, -10000.0f, 0.0f)
	));
}
