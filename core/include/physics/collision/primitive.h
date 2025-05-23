#pragma once
#include "../math.h"
#include "../rigid_body.h"

namespace phys {
	enum class shape_type {
		Sphere = 0,
		Plane,
		Box,
		// Not a primitive - indicates the end of known shape types. Consumers can implement
		// their own shape types with `type` starting at `shape_type::Max`
		Max
	};

	class primitive {
	public:
		const int type;
		rigid_body * body;
		// Offset from rigid body origin
		mat4 offset;

		virtual ~primitive() = default;

		const mat4& get_inv_offset() const;

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

	private:
		mat4 inv_offset;
	};
}
