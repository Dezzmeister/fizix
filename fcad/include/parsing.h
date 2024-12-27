#pragma once
#include <optional>
#include <data_formats/parsing.h>
#include <physics/collision/vclip.h>

using namespace phys;
using namespace phys::vclip;

struct partial_vec3 {
	std::optional<real> x{};
	std::optional<real> y{};
	std::optional<real> z{};

	std::optional<vec3> try_as_vec3() const;
};

std::optional<real> parse_real(parsing::parser_state &state);
std::optional<size_t> parse_size(parsing::parser_state &state);
partial_vec3 parse_vec3(parsing::parser_state &state);
std::optional<face> parse_implicit_face(parsing::parser_state &state);
