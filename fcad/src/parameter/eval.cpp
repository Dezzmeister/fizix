#include "controllers/geometry.h"
#include "traits.h"
#include "parameter/eval.h"

func_defn::func_defn(
	const std::wstring &_name,
	const args_type_defn &_args_t,
	const expr_class &_ret_t,
	const body_defn &_body
) :
	name(_name),
	args_t(_args_t),
	ret_t(_ret_t),
	body(_body)
{}

eval_context::eval_context(
	const scalar_symbol_map * _scalars,
	const vertex_defn_map * _verts,
	const geometry_controller *_geom,
	const func_defn_map *_builtin_funcs
) :
	scalars(_scalars),
	verts(_verts),
	geom(_geom),
	builtin_funcs(_builtin_funcs)
{}

unknown_ident_error::unknown_ident_error(const std::wstring &_ident) :
	std::runtime_error("Unknown identifier: " + traits::to_string(_ident)),
	ident(_ident)
{}