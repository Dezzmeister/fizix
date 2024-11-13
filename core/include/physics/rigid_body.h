#pragma once
#include "math.h"

namespace phys {
	class rigid_body {
	public:
		vec3 pos{};
		vec3 vel{};
		vec3 acc{};
		quat rot{};
		vec3 ang_vel{};
		real linear_damping;
		real angular_damping;

		rigid_body();

		void set_mass(real mass);
		real get_mass() const;
		real get_inv_mass() const;
		bool has_finite_mass() const;
		void setup();
		void calculate_derived_data();
		void add_force(const vec3 &f);
		void add_force_at_world(const vec3 &f_world, const vec3 &at_world);
		void add_force_at_local(const vec3 &f_world, const vec3 &at_local);
		void integrate(real dt);

	private:
		mat4 local_to_world;
		mat3 inv_inertia_tensor;
		mat3 inv_inertia_tensor_world;
		vec3 force{};
		vec3 torque{};
		real inv_mass;

		void calculate_local_to_world();
		void calculate_inv_inertia_tensor_world();
	};
}
