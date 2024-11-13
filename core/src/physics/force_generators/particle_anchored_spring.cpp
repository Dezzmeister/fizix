#include "physics/particle_force_generators.h"

phys::particle_anchored_spring::particle_anchored_spring(const vec3 &_anchor, real _k, real _rest_length) :
	particle_force_generator(),
	anchor(_anchor),
	k(_k),
	rest_length(_rest_length)
{}

void phys::particle_anchored_spring::update_force(particle &p, real) {
	vec3 d = p.pos - anchor;
	real d_len = sqrt(dot(d, d));

	vec3 f = -k * (d_len - rest_length) * normalize(d);
	p.force += f;
}