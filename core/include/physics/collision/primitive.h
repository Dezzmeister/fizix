#pragma once
#include "../math.h"
#include "../rigid_body.h"

namespace phys {
	enum class shape_type {
		Sphere = 0
	};

	class primitive {
	public:
		const int type;
		rigid_body * body;
		// Offset from rigid body origin
		mat4 offset;

		virtual ~primitive() = default;

	protected:
		primitive(
			shape_type _type,
			rigid_body * _body,
			const mat4 &_offset
		);

		primitive(
			int _type,
			rigid_body * _body,
			const mat4 &_offset
		);
	};
}
