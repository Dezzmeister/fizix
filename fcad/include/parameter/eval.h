#pragma once
#include <string>
#include <map>
#include <memory>
#include <stdexcept>
#include <vector>
#include "ast.h"

class scalar_expr;
struct scalar_parameter {
	std::unique_ptr<scalar_expr> defn{};
	std::vector<size_t> refs{};
};

using scalar_symbol_map = std::map<std::wstring, scalar_parameter>;

struct eval_context {
	const scalar_symbol_map * scalars{};
};

class unknown_ident_error : public std::runtime_error {
public:
	const std::wstring ident;

	unknown_ident_error(const std::wstring &_ident);
};
