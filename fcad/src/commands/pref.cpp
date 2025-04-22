#include "commands.h"
#include "helpers.h"
#include "parameter/parser.h"
#include "parsing.h"

void pref_command_impl::on_submit(const std::wstring &args) {
	std::wstringstream wss{};
	std::wstringstream sink{};
	wss << args;
	parsing::parser_state state(wss);
	error_log log{};

	parsing::parse_whitespace(state);

	std::optional<std::wstring> ident_opt = parse_bare_ident(state, log);

	if (! ident_opt) {
		if (log.errors.empty()) {
			log.errors.push_back(parse_error(
				L"Expected identifier",
				state.get_col_num()
			));
		}

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

	if (*ident_opt == L"log_output") {
		std::optional<std::wstring> val_opt = parse_bare_ident(state, log);

		if (! val_opt || (*val_opt != L"on" && *val_opt != L"off")) {
			log.errors.push_back(parse_error(
				L"Expected \"on\" or \"off\"",
				state.get_col_num()
			));
			set_output(log.to_wstr(args));
			return;
		}

		if (has_trailing_chars(state, args)) {
			return;
		}

		prefs->set_log_cmd_output(*val_opt == L"on");
		history->add_command(L":pref " + args);
	} else {
		log.errors.push_back(parse_error(
			L"Expected a preference name",
			state.get_col_num()
		));
		set_output(log.to_wstr(args));
		return;
	}
}

void pref_command_impl::write_help_text(std::ostream &os) const {
	write_help_rtf_row(os, ":pref",
		"TODO"
	);
}

bool pref_command_impl::has_trailing_chars(
	parsing::parser_state &state,
	const std::wstring &args
) const {
	parsing::parse_whitespace(state);

	if (! state.eof()) {
		error_log log{};
		log.errors.push_back(parse_error(
			L"Unexpected trailing chars",
			state.get_col_num()
		));
		set_output(log.to_wstr(args));
		return true;
	}

	return false;
}