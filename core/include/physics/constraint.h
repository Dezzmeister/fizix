#pragma once
#include <array>
#include <memory>
#include <vector>
#include "math.h"
#include "particle.h"

namespace phys {
	enum class constraint_type {
		Equality,
		Inequality
	};

	class constraint {
	public:
		real stiffness;
		constraint_type type;

		constraint(real _stiffness, constraint_type _type);
		virtual ~constraint() = default;

		virtual real eval_constraint() const = 0;

		virtual void project(real inv_solver_iterations) = 0;
		virtual void update_velocities(real dt) = 0;

		bool is_satisfied() const;
	};

	class constraint_generator {
	public:
		virtual ~constraint_generator() = default;

		virtual void generate_constraints(real dt, std::vector<std::unique_ptr<constraint>> &constraints) = 0;
	};

	template <const size_t N>
	class particle_constraint : public constraint {
	public:
		std::array<particle *, N> particles;

		particle_constraint(
			real _stiffness,
			constraint_type _type,
			std::array<particle *, N> _particles
		);
		virtual ~particle_constraint() = default;

		virtual real eval_constraint() const = 0;
		virtual vec3 eval_gradient(const particle &p) const = 0;
		void project(real inv_solver_iterations) override;
		virtual void update_velocities(real dt) = 0;

		phys::particle * a() requires (N >= 1);
		phys::particle * b() requires (N >= 2);

		const phys::particle * a() const requires (N >= 1);
		const phys::particle * b() const requires (N >= 2);

	private:
		std::array<vec3, N> dps{};
	};
}

template <const size_t N>
phys::particle_constraint<N>::particle_constraint(
	real _stiffness,
	constraint_type _type,
	std::array<particle *, N> _particles
) :
	constraint(_stiffness, _type),
	particles(_particles)
{}

template <const size_t N>
void phys::particle_constraint<N>::project(real inv_solver_iterations) {
	using namespace phys::literals;

	if constexpr (N == 0) {
		return;
	}

	real denom = 0.0_r;
	real k = (stiffness == 1.0_r) ? 1.0_r : 1 - std::pow((1 - stiffness), inv_solver_iterations);

	for (size_t i = 0; i < N; i++) {
		particle &p = *particles[i];
		vec3 grad = eval_gradient(p);

		denom += (p.get_inv_mass() * phys::dot(grad, grad));
		dps[i] = k * p.get_inv_mass() * grad;
	}

	if (denom == 0.0_r) {
		return;
	}

	real s = eval_constraint() / denom;

	for (size_t i = 0; i < N; i++) {
		dps[i] *= -s;
		particles[i]->p += dps[i];
	}
}

template <const size_t N>
phys::particle * phys::particle_constraint<N>::a() requires (N >= 1) {
	return particles[0];
}

template <const size_t N>
phys::particle * phys::particle_constraint<N>::b() requires (N >= 2) {
	return particles[1];
}

template <const size_t N>
const phys::particle * phys::particle_constraint<N>::a() const requires (N >= 1) {
	return particles[0];
}

template <const size_t N>
const phys::particle * phys::particle_constraint<N>::b() const requires (N >= 2) {
	return particles[1];
}
