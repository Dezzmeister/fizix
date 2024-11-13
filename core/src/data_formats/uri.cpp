#include <optional>
#include "data_formats/parsing.h"
#include "data_formats/uri.h"

namespace {
	size_t parse_pct_encoded(parsing::parser_state &state, std::wstringstream &out) {
		if (state.peek() != L'%') {
			return 0L;
		}

		state.get();

		if (! parsing::parse_hexdig(state, out)) {
			throw uri_error("Expected a hex digit", state.get_col_num());
		}

		if (! parsing::parse_hexdig(state, out)) {
			throw uri_error("Expected a hex digit", state.get_col_num());
		}

		return 3L;
	}

	std::wstring parse_scheme(parsing::parser_state &state) {
		std::wstringstream out{};

		if (! parsing::parse_alpha(state, out)) {
			throw uri_error("Scheme must begin with a letter", state.get_col_num());
		}

		while (
			parsing::parse_alpha(state, out) ||
			parsing::parse_digit(state, out) ||
			parsing::parse_one_char(state, L'+', out) ||
			parsing::parse_one_char(state, L'-', out) ||
			parsing::parse_one_char(state, L'.', out)
			);

		return out.str();
	}

	bool parse_unreserved(parsing::parser_state &state, std::wstringstream &out) {
		return (
			parsing::parse_alpha(state, out) ||
			parsing::parse_digit(state, out) ||
			parsing::parse_one_char(state, L'-', out) ||
			parsing::parse_one_char(state, L'.', out) ||
			parsing::parse_one_char(state, L'_', out) ||
			parsing::parse_one_char(state, L'~', out)
			);
	}

	// Despite the name, the `sub-delims` production will only match one terminal
	bool parse_sub_delims(parsing::parser_state &state, std::wstringstream &out) {
		return (
			parsing::parse_one_char(state, L'!', out) ||
			parsing::parse_one_char(state, L'$', out) ||
			parsing::parse_one_char(state, L'&', out) ||
			parsing::parse_one_char(state, L'\'', out) ||
			parsing::parse_one_char(state, L'(', out) ||
			parsing::parse_one_char(state, L')', out) ||
			parsing::parse_one_char(state, L'*', out) ||
			parsing::parse_one_char(state, L'+', out) ||
			parsing::parse_one_char(state, L',', out) ||
			parsing::parse_one_char(state, L';', out) ||
			parsing::parse_one_char(state, L'=', out)
			);
	}

	// Despite the name, the `gen-delims` production will only match one terminal
	bool parse_gen_delims(parsing::parser_state &state, std::wstringstream &out) {
		return (
			parsing::parse_one_char(state, L':', out) ||
			parsing::parse_one_char(state, L'/', out) ||
			parsing::parse_one_char(state, L'?', out) ||
			parsing::parse_one_char(state, L'#', out) ||
			parsing::parse_one_char(state, L'[', out) ||
			parsing::parse_one_char(state, L']', out) ||
			parsing::parse_one_char(state, L'@', out)
			);
	}

	bool parse_reg_name(parsing::parser_state &state, std::wstringstream &out) {
		bool parsed = false;

		while (parse_unreserved(state, out) || parse_pct_encoded(state, out) || parse_sub_delims(state, out)) {
			parsed = true;
		}

		return parsed;
	}

	std::optional<std::wstring> parse_userinfo(parsing::parser_state &state) {
		std::wstringstream out{};

		while (
			parse_unreserved(state, out) ||
			parse_pct_encoded(state, out) ||
			parse_sub_delims(state, out) ||
			parsing::parse_one_char(state, L':', out)
			);

		std::wstring str_out = out.str();

		return str_out;
	}

	std::wstringstream read_full_authority(parsing::parser_state &state) {
		std::wstringstream out{};

		while (! state.eof()) {
			wchar_t c = state.peek();

			switch (c) {
				case L'/':
				case L'?':
				case L'#':
					return out;
				default:
					out << state.get();
			}
		}

		return out;
	}

	uri_authority parse_authority(parsing::parser_state &state) {
		size_t pos_offset = state.get_col_num();
		std::wstringstream full_auth = read_full_authority(state);
		std::wstring full_auth_str = full_auth.str();
		full_auth.clear();
		full_auth.seekg(0);
		parsing::parser_state auth_state(full_auth);
		std::optional<std::wstring> userinfo{};

		size_t at_pos = full_auth_str.find(L'@');

		if (at_pos != std::wstring::npos) {
			userinfo = parse_userinfo(auth_state);

			if (auth_state.peek() != L'@') {
				throw uri_error("Invalid userinfo", pos_offset + auth_state.get_col_num());
			}

			auth_state.get();
		}

		std::wstringstream out{};
		uri_host host{};

		if (auth_state.peek() == L'[') {
			auth_state.get();

			std::optional<ipv6_addr> ip = parse_ipv6(auth_state, out);

			if (ip.has_value() && auth_state.peek() == L']') {
				host = *ip;
				auth_state.get();
			} else {
				throw uri_error("Invalid or missing IPv6 address", pos_offset + auth_state.get_col_num());
			}
		} else {
			std::optional<ipv4_addr> ip = parse_ipv4(auth_state, out);

			if (ip.has_value()) {
				host = *ip;
			}

			if (parse_reg_name(auth_state, out)) {
				host = out.str();
			}
		}

		std::optional<uint16_t> port;

		if (auth_state.peek() == L':') {
			auth_state.get();

			std::wstringstream digits{};

			while (parsing::parse_digit(auth_state, digits));

			if (! auth_state.eof()) {
				throw uri_error("Invalid port", pos_offset + auth_state.get_col_num());
			}

			uint64_t wide_port;
			digits >> wide_port;

			if (wide_port > 65535) {
				throw uri_error("Port must not be > 65535", pos_offset + auth_state.get_col_num());
			}

			port = (uint16_t)wide_port;
		} else if (auth_state.peek() != WEOF) {
			throw uri_error("Invalid char", pos_offset + auth_state.get_col_num());
		}

		return {
			.userinfo = userinfo,
			.host = host,
			.port = port
		};
	}
}

uri_error::uri_error(const std::string &message, size_t _char_pos) :
	std::runtime_error("Error parsing URI (col " + std::to_string(_char_pos) + "): " + message),
	char_pos(_char_pos)
{}

uri parse_uri(std::wistream &_in) {
	parsing::parser_state state(_in);

	std::wstring scheme = parse_scheme(state);
	uri_authority authority{};

	if (state.peek() != L':') {
		throw uri_error("Colon missing after scheme", state.get_col_num());
	}

	std::wstringstream path_and_rest{};

	state.get();

	if (state.peek() == L'/') {
		state.get();

		if (state.peek() == L'/') {
			state.get();
			authority = parse_authority(state);
		} else {
			path_and_rest << L'/';
		}
	}

	std::wstringstream drained = state.drain();
	path_and_rest << drained.rdbuf();

	return {
		.scheme = scheme,
		.authority = authority,
		.path_and_rest = path_and_rest.str()
	};
}
