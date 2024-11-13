#pragma once
#include "constraint.h"

namespace phys {
	class distance_constraint : public particle_constraint<2> {
	public:
		real distance;

		distance_constraint(
			particle * _a,
			particle * _b,
			real _distance,
			real _stiffness
		);

		real eval_constraint() const override;
		vec3 eval_gradient(const particle &p) const override;
		void update_velocities(real dt) override;
	};

	class plane_collision_constraint : public particle_constraint<1> {
	public:
		vec3 normal;
		vec3 origin;
		real restitution;
		real friction;

		plane_collision_constraint(
			particle * _a,
			vec3 _normal,
			vec3 _origin,
			real _restitution,
			real _friction
		);

		real eval_constraint() const override;
		vec3 eval_gradient(const particle &p) const override;
		void update_velocities(real dt) override;

	private:
		vec3 old_vel;
	};

	template <typename particle_container>
	class plane_collision_constraint_generator : public constraint_generator {
	public:
		plane_collision_constraint_generator(
			particle_container &_particles,
			vec3 _normal,
			vec3 _origin,
			real _restitution,
			real _friction
		);

		void generate_constraints(real dt, std::vector<std::unique_ptr<constraint>> &constraints) override;

	private:
		particle_container &particles;
		vec3 normal;
		vec3 origin;
		real restitution;
		real friction;
	};

	class particle_collision_constraint : public particle_constraint<2> {
	public:
		real restitution;
		real friction;

		particle_collision_constraint(
			particle * _a,
			particle * _b,
			real _restitution,
			real _friction
		);

		real eval_constraint() const override;
		vec3 eval_gradient(const particle &p) const override;
		void update_velocities(real dt) override;

	private:
		vec3 a_old_vel;
		vec3 b_old_vel;
		vec3 a_old_pos;
		vec3 b_old_pos;
	};
}

template <typename particle_container>
phys::plane_collision_constraint_generator<particle_container>::plane_collision_constraint_generator(
	particle_container &_particles,
	vec3 _normal,
	vec3 _origin,
	real _restitution,
	real _friction
) :
	particles(_particles),
	normal(_normal),
	origin(_origin),
	restitution(_restitution),
	friction(_friction)
{}

template <typename particle_container>
void phys::plane_collision_constraint_generator<particle_container>::generate_constraints(
	real dt,
	std::vector<std::unique_ptr<constraint>> &constraints
) {
	using namespace phys::literals;

	for (particle &p : particles) {
		if (phys::dot(p.p - origin, normal) < 0.0_r) {
			constraints.push_back(std::make_unique<phys::plane_collision_constraint>(
				&p,
				normal,
				origin,
				restitution,
				friction
			));
		}
	}
}
