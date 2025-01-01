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

using index_feature = std::variant<size_t, edge, face>;

std::optional<real> parse_real(parsing::parser_state &state);
std::optional<size_t> parse_size(parsing::parser_state &state);
partial_vec3 parse_vec3(parsing::parser_state &state);
std::optional<face> parse_implicit_face(parsing::parser_state &state);
std::wstring parse_line(parsing::parser_state &state);

// Each of these "explicit" functions expects the lookahead to match a distinct
// character (either '(', 'v', 'e', or 'f'). The input stream will only be mutated
// if the lookahead matches the marker character.
std::optional<vec3> parse_explicit_vec3(parsing::parser_state &state);
std::optional<size_t> parse_explicit_vertex(parsing::parser_state &state);
std::optional<edge> parse_explicit_edge(parsing::parser_state &state);
std::optional<face> parse_explicit_face(parsing::parser_state &state);

std::optional<index_feature> parse_explicit_feature(parsing::parser_state &state);
