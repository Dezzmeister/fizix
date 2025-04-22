#include "commands.h"
#include "controllers/parameter.h"
#include "helpers.h"
#include "parameter/parser.h"

void set_command_impl::on_submit(const std::wstring &args) {
	std::wstringstream wss{};
	std::wstringstream sink{};
	wss << args;
	parsing::parser_state state(wss);
	error_log log{};

	parsing::parse_whitespace(state);

	std::optional<size_t> vert_idx_opt = parse_explicit_vertex(state);
	std::optional<std::wstring> s_ident_opt = parse_scalar_ident(state, log);
	std::optional<std::wstring> v_ident_opt = parse_vector_ident(state, log);

	if (! vert_idx_opt && ! s_ident_opt && ! v_ident_opt) {
		set_output(log.to_wstr(args));
		return;
	}

	parsing::parse_whitespace(state);

	if (! parsing::parse_one_char(state, L'=', sink)) {
		log.errors.push_back(parse_error(
			L"Expected '='",
			state.get_col_num()
		));
		set_output(log.to_wstr(args));
		return;
	}

	parsing::parse_whitespace(state);

	std::optional<std::unique_ptr<scalar_expr>> scalar_expr_opt{};
	std::optional<std::unique_ptr<vector_expr>> vector_expr_opt{};

	if (s_ident_opt) {
		scalar_expr_opt = parse_scalar_expr(state, log);

		if (! scalar_expr_opt) {
			set_output(log.to_wstr(args));
			return;
		}
	} else {
		vector_expr_opt = parse_vector_expr(state, log);

		if (! vector_expr_opt) {
			set_output(log.to_wstr(args));
			return;
		}
	}

	parsing::parse_whitespace(state);

	if (! state.eof()) {
		log.errors.push_back(parse_error(
			L"Unexpected trailing chars",
			state.get_col_num()
		));
		set_output(log.to_wstr(args));
		return;
	}

	if (s_ident_opt || v_ident_opt) {
		std::wstring ident = s_ident_opt ? *s_ident_opt : *v_ident_opt;

		type_err_log type_errs = s_ident_opt ?
			params->typecheck(**scalar_expr_opt) :
			params->typecheck(**vector_expr_opt);

		if (! type_errs.errors.empty()) {
			set_output(type_errs.to_wstr());
			return;
		}

		try {
			if (s_ident_opt) {
				params->set_scalar_parameter(ident, std::move(*scalar_expr_opt));
			} else {
				params->set_vector_parameter(ident, std::move(*vector_expr_opt));
			}
		} catch (const param_does_not_exist_error&) {
			set_output(L"Parameter \"" + ident + L"\" does not exist");
			return;
		}
	} else {
		assert(vert_idx_opt);

		type_err_log type_errs = params->typecheck(**vector_expr_opt);

		if (! type_errs.errors.empty()) {
			set_output(type_errs.to_wstr());
			return;
		}

		if (! params->bind_vertex(*vert_idx_opt, std::move(*vector_expr_opt))) {
			return;
		}
	}

	history->add_command(L":set " + args);
}

void set_command_impl::write_help_text(std::ostream &os) const {
	write_help_rtf_row(os, "set",
		"TODO"
	);
}
