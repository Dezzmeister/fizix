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
	std::optional<std::wstring> ident_opt = parse_scalar_ident(state, log);

	if (! vert_idx_opt && ! ident_opt) {
		platform->set_cue_text(log.to_wstr(args));
		return;
	}

	parsing::parse_whitespace(state);

	if (! parsing::parse_one_char(state, L'=', sink)) {
		log.errors.push_back(parse_error(
			L"Expected '='",
			state.get_col_num()
		));
		platform->set_cue_text(log.to_wstr(args));
		return;
	}

	parsing::parse_whitespace(state);

	std::optional<std::unique_ptr<scalar_expr>> scalar_expr_opt{};
	std::optional<std::unique_ptr<vector_expr>> vector_expr_opt{};

	if (ident_opt) {
		scalar_expr_opt = parse_scalar_expr(state, log);

		if (! scalar_expr_opt) {
			platform->set_cue_text(log.to_wstr(args));
			return;
		}
	} else {
		assert(vert_idx_opt);

		vector_expr_opt = parse_vector_expr(state, log, false);

		if (! vector_expr_opt) {
			platform->set_cue_text(log.to_wstr(args));
			return;
		}
	}

	parsing::parse_whitespace(state);

	if (! state.eof()) {
		log.errors.push_back(parse_error(
			L"Unexpected trailing chars",
			state.get_col_num()
		));
		platform->set_cue_text(log.to_wstr(args));
		return;
	}

	if (ident_opt) {
		try {
			params->set_scalar_parameter(*ident_opt, std::move(*scalar_expr_opt));
		} catch (const param_does_not_exist_error&) {
			platform->set_cue_text(L"Parameter \"" + *ident_opt + L"\" does not exist");
			return;
		}
	} else {
		assert(vert_idx_opt);

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
