#include <physics/math.h>
#include "parsing.h"

std::optional<phys::real> parse_real(parsing::parser_state &state) {
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

	phys::real out;
	sink >> out;

	return out;
}