#include "physics/particle_force_generators.h"
#include "math.h"

phys::particle_drag::particle_drag(real _k1, real _k2) :
	particle_force_generator(),
	k1(_k1),
	k2(_k2)
{}

void phys::particle_drag::update_force(particle &p, real) {
	vec3 norm_vel = normalize(p.vel);
	real speed_sqr = dot(p.vel, p.vel);
	real speed = sqrt(speed_sqr);

	vec3 f = -norm_vel * (k1 * speed + k2 * speed_sqr);

	p.force += f;
}