#include <iostream>
#include "parameter/parser.h"
#include "util.h"

// Grammar
// S' -> + Sc S' | - Sc S' | epsilon
// S -> Lit S' | $ Ident S' | FE
// Sc -> (S) S' | S S'

// Vec -> (Ident {,} Ident {,} Ident) | vIdx

// FE -> Ident FE'
// FE' -> Sc FE' | Vec FE' | epsilon

namespace {
	const wchar_t NO_SIGIL = WEOF;
	const wchar_t SCALAR_SIGIL = L'$';

	std::optional<std::wstring> parse_ident(wchar_t sigil, parsing::parser_state &state) {
		std::wstringstream out{};

		if (sigil != NO_SIGIL) {
			out << sigil;
		}

		wchar_t c = state.peek();

		if (
			(c >= L'A' && c <= L'Z') ||
			(c >= L'a' && c <= L'z') ||
			c == L'_'
		) {
			out << state.get();
			c = state.peek();
		} else {
			return std::nullopt;
		}

		while (
			(c >= L'A' && c <= L'Z') ||
			(c >= L'a' && c <= L'z') ||
			c == L'_' ||
			(c >= L'0' && c <= L'9')
		) {
			out << state.get();
			c = state.peek();
		}

		std::wstring ident = out.str();

		if (sigil != NO_SIGIL && ident.size() == 1) {
			// We only have the sigil
			return std::nullopt;
		}

		return ident;
	}

	std::optional<std::unique_ptr<scalar_expr>> parse_scalar_p(
		parsing::parser_state &state,
		error_log &log,
		std::unique_ptr<scalar_expr> &prev_s
	) {
		wchar_t op = state.peek();

		if (op == L'+' || op == L'-' || op == L'*' || op == L'/') {
			state.get();
		} else {
			// epsilon
			return std::nullopt;
		}

		parsing::parse_whitespace(state);

		std::optional<std::unique_ptr<scalar_expr>> op2_opt = parse_scalar_expr(state, log);

		if (! op2_opt) {
			log.errors.push_back(parse_error(
				L"Expected scalar expression",
				state.get_col_num()
			));
			return std::nullopt;
		}

		parsing::parse_whitespace(state);

		std::unique_ptr<scalar_expr> new_op1{};

		switch (op) {
			case L'+': {
				new_op1 = std::make_unique<scalar_add_expr>(std::move(prev_s), std::move(*op2_opt));
				break;
			}
			case L'-': {
				new_op1 = std::make_unique<scalar_sub_expr>(std::move(prev_s), std::move(*op2_opt));
				break;
			}
			case L'*': {
				new_op1 = std::make_unique<scalar_mul_expr>(std::move(prev_s), std::move(*op2_opt));
				break;
			}
			case L'/': {
				new_op1 = std::make_unique<scalar_div_expr>(std::move(prev_s), std::move(*op2_opt));
				break;
			}
			default:
				std::unreachable();
		}

		std::optional<std::unique_ptr<scalar_expr>> sp = parse_scalar_p(state, log, new_op1);

		if (sp) {
			return sp;
		}

		return new_op1;
	}

	std::optional<std::unique_ptr<vector_literal_expr>> parse_vector_literal(
		parsing::parser_state &state,
		error_log &log,
		bool allow_implicit
	) {
		std::wstringstream sink{};
		bool has_open_paren = false;

		if (! allow_implicit) {
			if (! parsing::parse_one_char(state, L'(', sink)) {
				log.errors.push_back(parse_error(
					L"Expected '('",
					state.get_col_num()
				));
				return std::nullopt;
			}
		} else {
			has_open_paren = parsing::parse_one_char(state, L'(', sink);
		}

		parsing::parse_whitespace(state);
		std::optional<std::unique_ptr<scalar_expr>> x_opt = parse_scalar_expr(state, log);

		if (! x_opt) {
			log.errors.push_back(parse_error(
				L"Expected scalar",
				state.get_col_num()
			));
			return std::nullopt;
		}

		parsing::parse_whitespace(state);
		parsing::parse_one_char(state, L',', sink);
		parsing::parse_whitespace(state);

		std::optional<std::unique_ptr<scalar_expr>> y_opt = parse_scalar_expr(state, log);

		if (! y_opt) {
			log.errors.push_back(parse_error(
				L"Expected scalar",
				state.get_col_num()
			));
			return std::nullopt;
		}

		parsing::parse_whitespace(state);
		parsing::parse_one_char(state, L',', sink);
		parsing::parse_whitespace(state);

		std::optional<std::unique_ptr<scalar_expr>> z_opt = parse_scalar_expr(state, log);

		if (! z_opt) {
			log.errors.push_back(parse_error(
				L"Expected scalar",
				state.get_col_num()
			));
			return std::nullopt;
		}

		parsing::parse_whitespace(state);

		if (! allow_implicit) {
			if (! parsing::parse_one_char(state, L')', sink)) {
				log.errors.push_back(parse_error(
					L"Expected ')'",
					state.get_col_num()
				));
				return std::nullopt;
			}
		} else {
			bool has_close_paren = parse_one_char(state, L')', sink);

			if (has_open_paren && ! has_close_paren) {
				log.errors.push_back(parse_error(
					L"Expected ')'",
					state.get_col_num()
				));
				return std::nullopt;
			} else if (! has_open_paren && has_close_paren) {
				log.errors.push_back(parse_error(
					L"Unexpected ')'",
					state.get_col_num()
				));
				return std::nullopt;
			}
		}

		return std::make_unique<vector_literal_expr>(
			std::move(*x_opt),
			std::move(*y_opt),
			std::move(*z_opt)
		);
	}

	std::vector<std::unique_ptr<expr>> parse_fe_p(
		parsing::parser_state &state,
		error_log &log
	) {
		std::optional<std::unique_ptr<expr>> head_opt = parse_vector_expr(state, log, false);

		if (! head_opt) {
			head_opt = parse_scalar_expr(state, log);
		}

		if (! head_opt) {
			return {};
		}

		std::vector<std::unique_ptr<expr>> rest = parse_fe_p(state, log);
		std::vector<std::unique_ptr<expr>> out{};

		out.push_back(std::move(*head_opt));

		for (auto &&arg : rest) {
			out.push_back(std::move(arg));
		}

		return out;
	}

	std::optional<std::unique_ptr<scalar_func_expr>> parse_fe(
		parsing::parser_state &state,
		error_log &log
	) {
		std::optional<std::wstring> ident_opt = parse_ident(NO_SIGIL, state);

		if (! ident_opt) {
			return std::nullopt;
		}

		parsing::parse_whitespace(state);

		std::vector<std::unique_ptr<expr>> args = parse_fe_p(state, log);

		return std::make_unique<scalar_func_expr>(*ident_opt, std::move(args));
	}

	std::optional<std::unique_ptr<scalar_expr>> parse_s(
		parsing::parser_state &state,
		error_log &log
	) {
		std::optional<std::unique_ptr<scalar_expr>> op1_opt{};

		parsing::parse_whitespace(state);

		if (state.peek() == SCALAR_SIGIL) {
			std::optional<std::wstring> ident_opt = parse_scalar_ident(state, log);

			if (! ident_opt) {
				log.errors.push_back(parse_error(
					L"Expected identifier",
					state.get_col_num()
				));
				return std::nullopt;
			}

			op1_opt = std::make_unique<scalar_ident_expr>(*ident_opt);
		} else {
			std::optional<real> lit_opt = parse_real(state);

			if (lit_opt) {
				op1_opt = std::make_unique<scalar_literal_expr>(*lit_opt);
			} else {
				op1_opt = parse_fe(state, log);
			}
		}

		if (! op1_opt) {
			log.errors.push_back(parse_error(
				L"Expected scalar expression",
				state.get_col_num()
			));
			return std::nullopt;
		}

		parsing::parse_whitespace(state);

		std::optional<std::unique_ptr<scalar_expr>> sp = parse_scalar_p(state, log, *op1_opt);

		if (! sp) {
			return op1_opt;
		}

		return sp;
	}
}

parse_error::parse_error(const std::wstring &_err, size_t _char_pos) :
	err(_err), char_pos(_char_pos)
{}

std::wstring parse_error::to_wstr() const {
	using traits::to_string;

	return L"(" + util::to_wstring(to_string(char_pos)) + L"): " + err;
}

std::wstring error_log::to_wstr(const std::wstring &src) const {
	if (errors.empty()) {
		return L"";
	}

	const parse_error &err = errors.back();
	const std::wstring preview = src.substr(err.char_pos, src.size());

	return err.to_wstr() + L": " + preview;
}

std::optional<std::wstring> parse_bare_ident(
	parsing::parser_state &state,
	error_log&
) {
	return parse_ident(NO_SIGIL, state);
}

std::optional<std::wstring> parse_scalar_ident(
	parsing::parser_state &state,
	error_log &log
) {
	std::wstringstream sink{};

	if (! parsing::parse_one_char(state, SCALAR_SIGIL, sink)) {
		log.errors.push_back(parse_error(
			L"Expected '" + std::wstring(1, SCALAR_SIGIL) + L"'",
			state.get_col_num()
		));
		return std::nullopt;
	}

	return parse_ident(SCALAR_SIGIL, state);
}

std::optional<std::unique_ptr<scalar_expr>> parse_scalar_expr(
	parsing::parser_state &state,
	error_log &log
) {
	bool parens = false;

	parsing::parse_whitespace(state);

	if (state.peek() == L'(') {
		parens = true;
		state.get();
	}

	parsing::parse_whitespace(state);

	std::optional<std::unique_ptr<scalar_expr>> s = parse_s(state, log);

	if (! s) {
		return std::nullopt;
	}

	parsing::parse_whitespace(state);

	if (parens) {
		if (state.peek() == L')') {
			state.get();
		} else {
			log.errors.push_back(parse_error(
				L"Expected ')'",
				state.get_col_num()
			));
			return std::nullopt;
		}
	}

	parsing::parse_whitespace(state);

	std::optional<std::unique_ptr<scalar_expr>> sp_opt = parse_scalar_p(state, log, *s);

	if (sp_opt) {
		return sp_opt;
	}

	return s;
}

std::optional<std::unique_ptr<vector_expr>> parse_vector_expr(
	parsing::parser_state &state,
	error_log &log,
	bool allow_implicit_literals
) {
	return parse_vector_literal(state, log, allow_implicit_literals);
}