#include <physics/math.h>
#include "parsing.h"

std::optional<vec3> partial_vec3::try_as_vec3() const {
	if (! x || ! y || ! z) {
		return std::nullopt;
	}

	return vec3(*x, *y, *z);
}

std::optional<real> parse_real(parsing::parser_state &state) {
	std::wstringstream sink{};
	size_t num_digits = 0;

	parsing::parse_one_char(state, L'-', sink);
	num_digits += parsing::parse_dec_digits(state, sink);
	parsing::parse_one_char(state, L'.', sink);
	num_digits += parsing::parse_dec_digits(state, sink);

	if (num_digits == 0) {
		return std::nullopt;
	}

	parsing::parse_one_char(state, L'e', sink);
	parsing::parse_one_char(state, L'E', sink);
	parsing::parse_dec_digits(state, sink);

	real out;
	sink >> out;

	return out;
}

std::optional<size_t> parse_size(parsing::parser_state &state) {
	std::wstringstream sink{};

	size_t num_digits = parsing::parse_dec_digits(state, sink);

	if (num_digits == 0) {
		return std::nullopt;
	}

	size_t out;
	sink >> out;

	return out;
}

partial_vec3 parse_vec3(parsing::parser_state &state) {
	// TODO: Shared global null sink
	std::wstringstream sink{};
	std::optional<real> x = parse_real(state);

	parsing::parse_one_char(state, L',', sink);
	parsing::parse_whitespace(state);

	std::optional<real> y = parse_real(state);

	parsing::parse_one_char(state, L',', sink);
	parsing::parse_whitespace(state);

	std::optional<real> z = parse_real(state);

	return partial_vec3{
		.x = x,
		.y = y,
		.z = z
	};
}