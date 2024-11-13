#pragma once
#include "algorithm.h"
#include "contact.h"
#include "primitive.h"

namespace phys {
	struct collision_algorithm {
		int shape_type_1{};
		int shape_type_2{};
		collision_algorithm_func algorithm{};

		collision_algorithm(
			int _shape_type_1,
			int _shape_type_2,
			const collision_algorithm_func &_algorithm
		);

		collision_algorithm() = default;
	};

	class contact_generator {
	public:
		contact_generator();

		void generate_contacts(
			primitive &a,
			primitive &b,
			contact_container &contacts
		) const;

		void register_collision_algorithm(
			shape_type shape_type_1,
			shape_type shape_type_2,
			const collision_algorithm_func &algorithm
		);
		void register_collision_algorithm(
			shape_type shape_type_1,
			int shape_type_2,
			const collision_algorithm_func &algorithm
		);
		void register_collision_algorithm(
			int shape_type_1,
			int shape_type_2,
			const collision_algorithm_func &algorithm
		);
	private:
		static inline constexpr size_t max_shapes = 16;

		collision_algorithm algs[max_shapes][max_shapes]{};
	};
}
