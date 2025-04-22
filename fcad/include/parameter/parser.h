#pragma once
#include <vector>
#include "ast.h"
#include "parsing.h"

struct parse_error {
	const std::wstring err{};
	const size_t char_pos{};

	parse_error(const std::wstring &_err, size_t _char_pos);

	// TODO: to_wstring trait function
	std::wstring to_wstr() const;
};

class error_log {
public:
	std::vector<parse_error> errors{};

	// TODO: to_wstring trait function
	std::wstring to_wstr(const std::wstring &src) const;
};

std::optional<std::wstring> parse_bare_ident(
	parsing::parser_state &state,
	error_log &log
);

std::optional<std::wstring> parse_scalar_ident(
	parsing::parser_state &state,
	error_log &log
);
std::optional<std::unique_ptr<scalar_expr>> parse_scalar_expr(
	parsing::parser_state &state,
	error_log &log
);
std::optional<std::unique_ptr<vector_expr>> parse_vector_expr(
	parsing::parser_state &state,
	error_log &log,
	bool allow_implicit_literals = false
);
