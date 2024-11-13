#pragma once
#include "instanced_mesh.h"
#include "mesh.h"
#include "physics/particle.h"

extern void move_mesh_to_particle(mesh * m, const phys::particle * p);
extern void hide_particle(instanced_mesh * particles, size_t i);
