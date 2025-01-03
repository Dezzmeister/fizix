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

std::optional<face> parse_implicit_face(parsing::parser_state &state) {
	std::wstringstream sink{};
	std::vector<size_t> verts{};
	std::optional<size_t> i{};

	while ((i = parse_size(state))) {
		verts.push_back(*i);

		parsing::parse_one_char(state, L',', sink);
		parsing::parse_whitespace(state);
	}

	return face(verts, convexity::Unspecified);
}

std::wstring parse_line(parsing::parser_state &state) {
	std::wstringstream out{};

	wchar_t c;
	while (! state.eof()) {
		c = state.get();

		if (c == L'\n' || c == L'\r') {
			break;
		}

		out << c;
	}

	while (! state.eof()) {
		c = state.peek();

		if (c != L'\n' && c != L'\r') {
			break;
		}

		state.get();
	}

	return out.str();
}

std::optional<vec3> parse_explicit_vec3(parsing::parser_state &state) {
	if (state.peek() != L'(') {
		return std::nullopt;
	}

	state.get();

	parsing::parse_whitespace(state);

	std::optional<vec3> vec3_opt = parse_vec3(state).try_as_vec3();

	if (! vec3_opt) {
		return std::nullopt;
	}

	parsing::parse_whitespace(state);

	if (state.peek() != ')') {
		return std::nullopt;
	}

	state.get();

	return vec3_opt;
}

std::optional<size_t> parse_explicit_vertex(parsing::parser_state &state) {
	if (state.peek() != L'v') {
		return std::nullopt;
	}

	state.get();

	return parse_size(state);
}

std::optional<edge> parse_explicit_edge(parsing::parser_state &state) {
	if (state.peek() != L'e') {
		return std::nullopt;
	}

	state.get();

	// TODO: Shared null sink
	std::wstringstream sink{};

	if (! parsing::parse_one_char(state, L'(', sink)) {
		return std::nullopt;
	}

	parsing::parse_whitespace(state);

	std::optional<size_t> v1 = parse_size(state);

	if (! v1) {
		return std::nullopt;
	}

	parsing::parse_one_char(state, L',', sink);
	parsing::parse_whitespace(state);

	std::optional<size_t> v2 = parse_size(state);

	if (! v2) {
		return std::nullopt;
	}

	parsing::parse_whitespace(state);

	if (! parsing::parse_one_char(state, L')', sink)) {
		return std::nullopt;
	}

	return edge(*v1, *v2);
}

std::optional<face> parse_explicit_face(parsing::parser_state &state) {
	if (state.peek() != L'f') {
		return std::nullopt;
	}

	state.get();

	// TODO: Shared null sink
	std::wstringstream sink{};

	if (! parsing::parse_one_char(state, L'(', sink)) {
		return std::nullopt;
	}

	parsing::parse_whitespace(state);

	std::vector<size_t> verts{};
	std::optional<size_t> vi = parse_size(state);

	while (vi) {
		verts.push_back(*vi);

		parsing::parse_whitespace(state);
		vi = parse_size(state);
	}

	if (! parsing::parse_one_char(state, L')', sink)) {
		return std::nullopt;
	}

	return face(verts, convexity::Unspecified);
}

std::optional<index_feature> parse_explicit_feature(parsing::parser_state &state) {
	wchar_t c = state.peek();

	switch (c) {
		case L'v':
			return parse_explicit_vertex(state);
		case L'e':
			return parse_explicit_edge(state);
		case L'f':
			return parse_explicit_face(state);
		default:
			return std::nullopt;
	}
}

std::optional<vec3_or_index_feature> parse_explicit_vec3_or_feature(parsing::parser_state &state) {
	wchar_t c = state.peek();

	switch (c) {
		case L'(':
			return parse_explicit_vec3(state);
		// TODO: Dedup this and parse_explicit_feature
		case L'v':
			return parse_explicit_vertex(state);
		case L'e':
			return parse_explicit_edge(state);
		case L'f':
			return parse_explicit_face(state);
		default:
			return std::nullopt;
	}
}