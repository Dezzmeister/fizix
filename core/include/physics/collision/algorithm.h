#pragma once
#include <functional>
#include <vector>
#include "contact.h"
#include "primitive.h"

namespace phys {
	using contact_container = std::vector<contact>;
	// A collision algorithm func will receive its primitive arguments in
	// ascending order of `type`, so the first argument can be statically cast
	// to a reference to the shape with a lower `type`, and the second argument
	// can be statically cast to a reference to the shape with a higher `type`.
	using collision_algorithm_func = std::function<void(primitive&, primitive&, contact_container&)>;
}
