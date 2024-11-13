#pragma once
#include <functional>
#include <vector>
#include "contact.h"
#include "primitive.h"

namespace phys {
	using contact_container = std::vector<contact>;
	using collision_algorithm_func = std::function<void(primitive&, primitive&, contact_container&)>;
}
