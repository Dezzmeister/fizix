#include "controllers/file.h"
#include "parsing.h"

namespace {
	std::optional<vec3> parse_vertex(parsing::parser_state &state, std::wstringstream &sink) {
		parsing::parse_whitespace(state);

		if (! parsing::parse_string(state, L"vertex ", sink)) {
			logger::error(
				"Invalid STL file: expected \"vertex \" (line " +
				traits::to_string(state.get_line_num()) + ")"
			);

			return std::nullopt;
		}

		std::optional<vec3> v = parse_vec3(state).try_as_vec3();

		if (! v) {
			logger::error(
				"Invalid STL file: expected vertex (line " +
				traits::to_string(state.get_line_num()) + ")"
			);

			return std::nullopt;
		}

		return v;
	}

	std::optional<triangle> parse_facet(parsing::parser_state &state, std::wstringstream &sink) {
		parsing::parse_whitespace(state);

		if (! parsing::parse_string(state, L"facet normal ", sink)) {
			logger::error(
				"Invalid STL file: expected \"facet normal \" (line " +
				traits::to_string(state.get_line_num()) + ")"
			);

			return std::nullopt;
		}

		std::optional<vec3> normal = parse_vec3(state).try_as_vec3();

		if (! normal) {
			logger::error(
				"Invalid STL file: expected normal vector (line " +
				traits::to_string(state.get_line_num()) + ")"
			);

			return std::nullopt;
		}

		parsing::parse_whitespace(state);

		if (! parsing::parse_string(state, L"outer loop", sink)) {
			logger::error(
				"Invalid STL file: expected \"outer loop\" (line " +
				traits::to_string(state.get_line_num()) + ")"
			);

			return std::nullopt;
		}

		std::optional<vec3> v1 = parse_vertex(state, sink);

		if (! v1) {
			return std::nullopt;
		}

		std::optional<vec3> v2 = parse_vertex(state, sink);

		if (! v2) {
			return std::nullopt;
		}

		std::optional<vec3> v3 = parse_vertex(state, sink);

		if (! v3) {
			return std::nullopt;
		}

		parsing::parse_whitespace(state);

		if (! parsing::parse_string(state, L"endloop", sink)) {
			logger::error(
				"Invalid STL file: expected \"endloop\" (line " +
				traits::to_string(state.get_line_num()) + ")"
			);

			return std::nullopt;
		}

		parsing::parse_whitespace(state);

		if (! parsing::parse_string(state, L"endfacet", sink)) {
			logger::error(
				"Invalid STL file: expected \"endfacet\" (line " +
				traits::to_string(state.get_line_num()) + ")"
			);

			return std::nullopt;
		}

		triangle out{
			.v1 = *v1,
			.v2 = *v2,
			.v3 = *v3,
			.normal = *normal
		};

		return out;
	}
}

void file_controller::read_stl_file(const std::filesystem::path &path) {
	read_stl_file_event event(path);
	if (events.fire(event)) {
		logger::info("Cancelled reading STL file from " + path.string());

		return;
	}
	 
	logger::info("Reading STL file from " + path.string());

	std::wifstream wfs(path);
	parsing::parser_state state(wfs);
	std::wstringstream sink{};

	if (! parsing::parse_string(state, L"solid", sink)) {
		logger::error(
			"Invalid STL file: expected \"solid \" (line " +
			traits::to_string(state.get_line_num())
		);

		return;
	}

	parsing::parse_until(state, L'\n', sink);
	std::vector<triangle> facets{};

	while (! state.eof() && state.peek() != L'e') {
		std::optional<triangle> tri_opt = parse_facet(state, sink);
		if (! tri_opt) {
			return;
		}

		parsing::parse_whitespace(state);
		facets.push_back(*tri_opt);
	}

	if (! parsing::parse_string(state, L"endsolid", sink)) {
		logger::error(
			"Invalid STL file: expected \"endsolid\" (line " +
			traits::to_string(state.get_line_num())
		);

		return;
	}

	// Technically the name of the STL needs to be repeated after
	// "endsolid," but we don't care

	geom->reset();

	for (const triangle &tri : facets) {
		geom->add_triangle(tri);
	}

	geom->regenerate_edge_geom();
	edit_history->add_command(L":r " + path.wstring());
}
