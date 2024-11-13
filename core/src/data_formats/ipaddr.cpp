#include <vector>
#include "data_formats/ipaddr.h"

namespace {
	struct ipv4_rest {
		uint8_t rest[3];
	};

	std::optional<uint8_t> parse_dec_octet(parsing::parser_state &state, std::wstringstream &out) {
		wchar_t c0 = state.peek();
		int8_t d0 = parsing::parse_digit(state);

		if (d0 == -1) {
			return std::nullopt;
		}

		out << c0;

		wchar_t c1 = state.peek();

		if (d0 == 0 && parsing::is_digit(c1)) {
			return std::nullopt;
		}

		int8_t d1 = parsing::parse_digit(state);

		if (d1 == -1) {
			return d0;
		}

		out << c1;

		if (d0 == 0) {
			return std::nullopt;
		}

		wchar_t c2 = state.peek();
		int8_t d2 = parsing::parse_digit(state);

		if (d2 == -1) {
			return (uint8_t)((d0 * 10) + d1);
		}

		out << c2;

		uint16_t dec_out = (d0 * 100) + (d1 * 10) + d2;

		if (dec_out > 255) {
			return std::nullopt;
		}

		return (uint8_t)(dec_out & 0xFF);
	}

	std::optional<ipv4_rest> parse_ipv4_rest(parsing::parser_state &state, std::wstringstream &out) {
		std::optional<uint8_t> b1;
		std::optional<uint8_t> b2;
		std::optional<uint8_t> b3;

		parsing::parse_one_char(state, L'.', out);

		if (! (b1 = parse_dec_octet(state, out)).has_value()) {
			return std::nullopt;
		}

		parsing::parse_one_char(state, L'.', out);

		if (! (b2 = parse_dec_octet(state, out)).has_value()) {
			return std::nullopt;
		}

		parsing::parse_one_char(state, L'.', out);

		if (! (b3 = parse_dec_octet(state, out)).has_value()) {
			return std::nullopt;
		}

		return {
			{ *b1, *b2, *b3 }
		};
	}

	std::optional<uint16_t> parse_h16(parsing::parser_state &state, std::wstringstream &out) {
		wchar_t c0 = state.peek();
		int8_t d0 = parsing::parse_hexdig(state);

		if (d0 == -1) {
			return std::nullopt;
		}

		out << c0;

		wchar_t c1 = state.peek();
		int8_t d1 = parsing::parse_hexdig(state);

		if (d1 == -1) {
			return d0;
		}

		out << c1;

		wchar_t c2 = state.peek();
		int8_t d2 = parsing::parse_hexdig(state);

		if (d2 == -1) {
			return (uint16_t)((d0 << 4) | d1);
		}

		out << c2;

		wchar_t c3 = state.peek();
		int8_t d3 = parsing::parse_hexdig(state);

		if (d3 == -1) {
			return (uint16_t)((d0 << 8) | (d1 << 4) | d2);
		}

		out << c3;

		return (uint16_t)((d0 << 12) | (d1 << 8) | (d2 << 4) | d3);
	}

	void set_ls32(const ipv4_addr &ls32, ipv6_addr &out) {
		out.addr[6] = (ls32.addr[0] << 8) | ls32.addr[1];
		out.addr[7] = (ls32.addr[2] << 8) | ls32.addr[3];
	}

	uint16_t str_to_h16(const std::wstring &str) {
		std::wistringstream in_stream(str);
		parsing::parser_state state(in_stream);
		std::wstringstream null_out{};

		return parse_h16(state, null_out).value_or(0);
	}
}

ipv4_addr::ipv4_addr(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3) :
	addr{ b0, b1, b2, b3 }
{}

ipv6_addr::ipv6_addr(uint16_t b0, uint16_t b1, uint16_t b2, uint16_t b3,
	uint16_t b4, uint16_t b5, uint16_t b6, uint16_t b7) :
	addr{ b0, b1, b2, b3, b4, b5, b6, b7 }
{}

std::optional<ipv4_addr> parse_ipv4(parsing::parser_state &state, std::wstringstream &out) {
	std::optional<uint8_t> b0;

	if (! (b0 = parse_dec_octet(state, out)).has_value()) {
		return std::nullopt;
	}

	std::optional<ipv4_rest> rest = parse_ipv4_rest(state, out);

	if (! rest.has_value()) {
		return std::nullopt;
	}

	return {
		{ *b0, rest->rest[0], rest->rest[1], rest->rest[2] }
	};
}

std::optional<ipv6_addr> parse_ipv6(parsing::parser_state &state, std::wstringstream &out) {
	ipv6_addr addr_out{};

	wchar_t peek;
	std::wstring curr_h16{};
	size_t i = 0;
	std::vector<uint16_t> last_pieces{};
	bool reading_last = false;

	while ((peek = state.peek()) && i < 8) {
		if (peek == L':') {
			uint16_t h16 = str_to_h16(curr_h16);

			if (reading_last) {
				last_pieces.push_back(h16);
			} else {
				addr_out.addr[i++] = h16;
			}

			curr_h16 = L"";

			if (
				(! last_pieces.size() && (i + last_pieces.size() == 8)) ||
				(last_pieces.size() && (i + last_pieces.size() == 7))
				) {
				break;
			}

			out << state.get();

			if (state.peek() == L':') {
				out << state.get();
				reading_last = true;
			}
		} else if (parsing::is_hexdig(peek)) {
			curr_h16 += peek;

			if (curr_h16.size() > 4) {
				return std::nullopt;
			}

			out << state.get();
		} else if (peek == L'.') {
			if (i > 6) {
				return std::nullopt;
			}

			if (reading_last) {
				if (i + last_pieces.size() > 8) {
					return std::nullopt;
				}

				for (size_t j = 0; j < last_pieces.size(); j++) {
					addr_out.addr[6 - (last_pieces.size() - j)] = last_pieces[j];
				}
			}

			std::wistringstream ipv4_in(curr_h16);
			parsing::parser_state ipv4_state(ipv4_in);
			std::wstringstream null_out;

			std::optional<uint8_t> first_octet = parse_dec_octet(ipv4_state, null_out);

			if (! first_octet.has_value()) {
				return std::nullopt;
			}

			std::optional<ipv4_rest> rest = parse_ipv4_rest(state, out);

			if (! rest.has_value()) {
				return std::nullopt;
			}

			ipv4_addr ipv4(*first_octet, rest->rest[0], rest->rest[1], rest->rest[2]);
			set_ls32(ipv4, addr_out);

			return addr_out;
		} else {
			uint16_t h16 = str_to_h16(curr_h16);

			if (reading_last) {
				last_pieces.push_back(h16);
			} else {
				addr_out.addr[i++] = h16;
			}

			curr_h16 = L"";
			break;
		}
	}

	if (i == 8) {
		return addr_out;
	} else if (reading_last) {
		if (i + last_pieces.size() > 8) {
			return std::nullopt;
		}

		for (size_t j = 0; j < last_pieces.size(); j++) {
			addr_out.addr[8 - (last_pieces.size() - j)] = last_pieces[j];
		}

		return addr_out;
	}

	return std::nullopt;
}

bool operator==(const ipv4_addr &a, const ipv4_addr &b) {
	for (size_t i = 0; i < 4; i++) {
		if (a.addr[i] != b.addr[i]) {
			return false;
		}
	}

	return true;
}

bool operator==(const ipv6_addr &a, const ipv6_addr &b) {
	for (size_t i = 0; i < 8; i++) {
		if (a.addr[i] != b.addr[i]) {
			return false;
		}
	}

	return true;
}

template <>
std::string traits::to_string<ipv4_addr>(const ipv4_addr &addr, size_t) {
	return (
		std::to_string(addr.addr[0]) + "." +
		std::to_string(addr.addr[1]) + "." +
		std::to_string(addr.addr[2]) + "." +
		std::to_string(addr.addr[3])
		);
}

template <>
std::string traits::to_string<ipv6_addr>(const ipv6_addr&, size_t) {
	// TODO: Implement this
	throw "Unimplemented";
}