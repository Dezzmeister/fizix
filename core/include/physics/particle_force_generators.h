#pragma once
#include "particle_force_generator.h"

namespace phys {
	class particle_gravity : public particle_force_generator {
	public:
		vec3 gravity;

		particle_gravity(const vec3 &_gravity);

		void update_force(particle &p, real duration) override;
	};

	class particle_drag : public particle_force_generator {
	public:
		real k1;
		real k2;

		particle_drag(real _k1, real _k2);

		void update_force(particle &p, real duration) override;
	};

	class particle_spring : public particle_force_generator {
	public:
		const particle &other;
		real k;
		real rest_length;

		particle_spring(const particle &_other, real _k, real _rest_length);

		void update_force(particle &p, real duration) override;
	};

	class particle_anchored_spring : public particle_force_generator {
	public:
		vec3 anchor;
		real k;
		real rest_length;

		particle_anchored_spring(const vec3 &_anchor, real _k, real _rest_length);

		void update_force(particle &p, real duration) override;
	};
}
