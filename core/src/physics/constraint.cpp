#include "physics/constraint.h"

phys::constraint::constraint(real _stiffness, constraint_type _type) :
	stiffness(_stiffness),
	type(_type)
{}

bool phys::constraint::is_satisfied() const {
	using namespace phys::literals;

	switch (type) {
		case constraint_type::Equality:
			return eval_constraint() == 0.0_r;
		case constraint_type::Inequality:
			return eval_constraint() >= 0.0_r;
	}

	__assume(false);
}