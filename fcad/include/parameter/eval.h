#pragma once
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

using scalar_symbol_map = std::map<std::wstring, scalar_parameter>;
using vertex_defn_map = std::map<size_t, std::unique_ptr<vector_expr>>;

struct eval_context {
	const scalar_symbol_map * scalars{};
	const vertex_defn_map * verts{};
	const geometry_controller * geom{};

	eval_context(
		const scalar_symbol_map *_scalars,
		const vertex_defn_map * _verts,
		const geometry_controller * _geom
	);
};

class unknown_ident_error : public std::runtime_error {
public:
	const std::wstring ident;

	unknown_ident_error(const std::wstring &_ident);
};
