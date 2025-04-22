#pragma once
#include <functional>
#include <string>
#include <map>
#include <memory>
#include <stdexcept>
#include <vector>
#include "ast.h"

class geometry_controller;

struct scalar_parameter {
	std::unique_ptr<scalar_expr> defn{};
	std::vector<size_t> refs{};
};

struct vector_parameter {
	std::unique_ptr<vector_expr> defn{};
	std::vector<size_t> refs{};
};

using args_type_defn = std::vector<expr_class>;
using func_arg = std::variant<phys::real, phys::vec3>;
using args_obj = std::vector<func_arg>;
using body_defn = std::function<func_arg(const args_obj&)>;

struct func_defn {
	const std::wstring name;
	const args_type_defn args_t;
	const expr_class ret_t;
	const body_defn body;

	func_defn(
		const std::wstring &_name,
		const args_type_defn &_args_t,
		const expr_class &_ret_t,
		const body_defn &_body
	);
};

using scalar_symbol_map = std::map<std::wstring, scalar_parameter>;
using vector_symbol_map = std::map<std::wstring, vector_parameter>;
using vertex_defn_map = std::map<size_t, std::unique_ptr<vector_expr>>;
using func_defn_map = std::map<std::wstring, func_defn>;

struct eval_context {
	const scalar_symbol_map * scalars{};
	const vector_symbol_map * vectors{};
	const vertex_defn_map * verts{};
	const geometry_controller * geom{};
	const func_defn_map * builtin_funcs{};

	eval_context(
		const scalar_symbol_map *_scalars,
		const vector_symbol_map * _vectors,
		const vertex_defn_map * _verts,
		const geometry_controller * _geom,
		const func_defn_map *_builtin_funcs
	);
};

class unknown_ident_error : public std::runtime_error {
public:
	const std::wstring ident;

	unknown_ident_error(const std::wstring &_ident);
};

class undefined_vertex_error : public std::runtime_error {
public:
	const size_t vert_idx;

	undefined_vertex_error(size_t _vert_idx);
};
