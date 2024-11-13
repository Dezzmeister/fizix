#include "physics/particle_force_generators.h"

phys::particle_spring::particle_spring(const particle &_other, real _k, real _rest_length) :
	particle_force_generator(),
	other(_other),
	k(_k),
	rest_length(_rest_length)
{}

void phys::particle_spring::update_force(particle &p, real) {
	vec3 disp = p.pos - other.pos;
	real length = std::sqrt(dot(disp, disp));
	real dl = length - rest_length;
	vec3 f = -k * disp * dl;

	p.force += f;
}