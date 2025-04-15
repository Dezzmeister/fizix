#pragma once
#include "../../traits.h"
#include "../math.h"
#include "../rigid_body.h"
#include "vclip.h"

namespace phys {
	struct contact {
		rigid_body * a;
		rigid_body * b;

		vec3 point;
		vec3 normal;
		real penetration;

		contact(
			rigid_body * _a,
			rigid_body * _b,
			const vec3 &_point,
			const vec3 &_normal,
			real _penetration
		);

		static contact from_vclip(
			rigid_body * _a,
			rigid_body * _b,
			vclip::algorithm_result &_result
		);

		friend bool operator==(const contact &a, const contact &b);
	};
}

template <>
std::string traits::to_string(const phys::contact &c, size_t indent);
