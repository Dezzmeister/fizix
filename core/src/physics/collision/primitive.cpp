#include "physics/collision/primitive.h"

phys::primitive::primitive(
	shape_type _type,
	rigid_body * _body,
	const mat4 &_offset
) : primitive(static_cast<int>(_type), _body, _offset) {}

phys::primitive::primitive(
	int _type,
	rigid_body * _body,
	const mat4 &_offset
) :
	type(_type),
	body(_body),
	offset(_offset)
{}