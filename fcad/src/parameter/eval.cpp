#include "controllers/geometry.h"
#include "traits.h"
#include "parameter/eval.h"

eval_context::eval_context(
	const scalar_symbol_map * _scalars,
	const vertex_defn_map * _verts,
	const geometry_controller *_geom
) :
	scalars(_scalars),
	verts(_verts),
	geom(_geom)
{}

unknown_ident_error::unknown_ident_error(const std::wstring &_ident) :
	std::runtime_error("Unknown identifier: " + traits::to_string(_ident)),
	ident(_ident)
{}