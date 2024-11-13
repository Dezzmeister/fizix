#pragma once
#include <optional>
#include <variant>
#include "../traits.h"
#include "parsing.h"

struct ipv4_addr {
	uint8_t addr[4]{ 0 };

	ipv4_addr() = default;
	ipv4_addr(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3);

	friend bool operator==(const ipv4_addr &a, const ipv4_addr &b);
};

struct ipv6_addr {
	uint16_t addr[8]{ 0 };

	ipv6_addr() = default;
	ipv6_addr(uint16_t b0, uint16_t b1, uint16_t b2, uint16_t b3,
		uint16_t b4, uint16_t b5, uint16_t b6, uint16_t b7);

	friend bool operator==(const ipv6_addr &a, const ipv6_addr &b);
};

// Attempts to parse an IPv4 address. Returns the address if successful, otherwise returns
// std::nullopt. The parser will be left at the character after the valid IP address if one
// was present; otherwise, the parser will be left at the first invalid character. In either
// case, any consumed chars will be written to the output stream.
std::optional<ipv4_addr> parse_ipv4(parsing::parser_state &state, std::wstringstream &out);

// Attempts to parse an IPv6 address. Returns the address if successful, otherwise returns
// std::nullopt. The parser will be left at the character after the valid IP address if one
// was present; otherwise, the parser will be left at the first invalid character. In either
// case, any consumed chars will be written to the output stream.
std::optional<ipv6_addr> parse_ipv6(parsing::parser_state &state, std::wstringstream &out);

namespace traits {
	template <>
	std::string to_string<ipv4_addr>(const ipv4_addr &addr, size_t indent);

	template <>
	std::string to_string<ipv6_addr>(const ipv6_addr &addr, size_t indent);
}
