#include "physics/collision/algorithms.h"
#include "physics/collision/contact_generator.h"

phys::collision_algorithm::collision_algorithm(
	int _shape_type_1,
	int _shape_type_2,
	const collision_algorithm_func &_algorithm
) :
	shape_type_1(_shape_type_1),
	shape_type_2(_shape_type_2),
	algorithm(_algorithm)
{}

phys::contact_generator::contact_generator() {
	register_collision_algorithm(
		shape_type::Sphere,
		shape_type::Sphere,
		phys::algorithms::sphere_sphere_collision
	);
}

void phys::contact_generator::generate_contacts(
	primitive &a,
	primitive &b,
	contact_container &contacts
) const {
	if (a.type >= max_shapes || b.type >= max_shapes) {
		// TODO: Errors
		throw "Invalid shape type";
	}

	int shape_type_1 = std::min(a.type, b.type);
	int shape_type_2 = std::max(a.type, b.type);

	const collision_algorithm &alg = algs[shape_type_1][shape_type_2];

	if (! alg.algorithm) {
		// TODO: Errors
		throw "No collision algorithm";
	}

	alg.algorithm(a, b, contacts);
}

void phys::contact_generator::register_collision_algorithm(
	shape_type shape_type_1,
	shape_type shape_type_2,
	const collision_algorithm_func &algorithm
) {
	register_collision_algorithm(
		static_cast<int>(shape_type_1),
		static_cast<int>(shape_type_2),
		algorithm
	);
}

void phys::contact_generator::register_collision_algorithm(
	shape_type shape_type_1,
	int shape_type_2,
	const collision_algorithm_func &algorithm
) {
	register_collision_algorithm(
		static_cast<int>(shape_type_1),
		static_cast<int>(shape_type_2),
		algorithm
	);
}

void phys::contact_generator::register_collision_algorithm(
	int shape_type_1,
	int shape_type_2,
	const collision_algorithm_func &algorithm
) {
	int type_1 = std::min(shape_type_1, shape_type_2);
	int type_2 = std::max(shape_type_1, shape_type_2);

	algs[type_1][type_2] = collision_algorithm(type_1, type_2, algorithm);
}