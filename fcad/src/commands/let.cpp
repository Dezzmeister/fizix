#include "commands.h"
#include "helpers.h"
#include "parameter/parser.h"
#include "parsing.h"

void let_command_impl::on_submit(const std::wstring &args) {
	std::wstringstream wss{};
	std::wstringstream sink{};
	wss << args;
	parsing::parser_state state(wss);

	parsing::parse_whitespace(state);
	error_log log{};

	std::optional<std::wstring> ident_opt = parse_scalar_ident(state, log);

	if (! ident_opt) {
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

	std::optional<std::unique_ptr<scalar_expr>> expr = parse_scalar_expr(state, log);

	if (! expr) {
		set_output(log.to_wstr(args));
		return;
	}

	parsing::parse_whitespace(state);

	if (! state.eof()) {
		if (log.errors.empty()) {
			log.errors.push_back(parse_error(
				L"Unexpected trailing chars",
				state.get_col_num()
			));
		}
		set_output(log.to_wstr(args));
		return;
	}

	type_err_log type_errs = params->typecheck(**expr);

	if (! type_errs.errors.empty()) {
		set_output(type_errs.to_wstr());
		return;
	}

	try {
		params->add_scalar_parameter(*ident_opt, std::move(*expr));
	} catch (const param_exists_error&) {
		set_output(L"Parameter \"" + *ident_opt + L"\" already exists");
		return;
	}

	set_output(L"Created parameter \"" + *ident_opt + L"\"");
	history->add_command(L":let " + args);
}

void let_command_impl::write_help_text(std::ostream &os) const {
	write_help_rtf_row(os, ":let <ident> = <expr>",
		"Creates a parameter named <ident> and binds it to <expr>. "
		"(TODO: Write parameters section) See Parameters."
	);
}