#pragma once
#include <array>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <variant>
#include <vector>
#include "physics/math.h"
#include "traits.h"
#include "util.h"

namespace phys {
	// Implementation of the V-Clip algorithm for collision detection between
	// convex polyhedra as described in https://merl.com/publications/TR97-05.
	// Mitsubishi's patent on this algorithm expired in 2012:
	// https://patentcenter.uspto.gov/applications/08921162
	namespace vclip {
		struct vertex;
		struct edge;
		struct face;
		struct vplane;

		// A collection of features, defined with reference to a set of vertices
		struct polyhedron {
			const std::vector<vertex> vertices{};
			const std::vector<edge> edges{};
			const std::vector<face> faces{};

			int euler_characteristic() const;

			void validate() const;
			void validate_references() const;
			void validate_geometry() const;
		};

		template <typename T>
		concept feature_t =
			std::same_as<T, vertex> ||
			std::same_as<T, edge> ||
			std::same_as<T, face>;

		struct vertex {
			vec3 v;
			// This should be the vertex's position in the polyhedron's
			// vertex array
			size_t i;

			vertex(const vec3 &_v, size_t _i);

			// Returns a view that will yield neighboring edges
			std::ranges::view auto edges(const polyhedron &p) const &;
			std::ranges::view auto edges(const polyhedron &p) const && = delete;

			std::vector<vplane> ve_planes(const polyhedron &p) const;

			friend bool operator==(const vertex &v1, const vertex &v2);
		};

		struct edge {
			// These are the positions of the edge's vertices in the polyhedron's
			// vertex array
			std::array<size_t, 2> v_is;

			edge(size_t _t_i, size_t _h_i);

			const vertex& t(const polyhedron &p) const;
			const vertex& h(const polyhedron &p) const;

			// Returns a view that will yield neighboring vertices
			std::ranges::view auto vertices(const polyhedron &p) const &;
			std::ranges::view auto vertices(const polyhedron &p) const && = delete;
			// Returns a view that will yield neighboring faces
			std::ranges::view auto faces(const polyhedron &p) const &;
			std::ranges::view auto faces(const polyhedron &p) const && = delete;

			std::vector<vplane> ve_planes(const polyhedron &p) const;
			std::vector<vplane> fe_planes(const polyhedron &p) const;

			bool has_vertex(size_t i) const;

			vec3 at(const polyhedron &p, real l) const;

			friend bool operator==(const edge &e1, const edge &e2);
		};

		struct face {
			// These are the positions of the face's vertices in the polyhedron's
			// vertex array. The winding order of the vertices determines the
			// orientation of the face; the face normal points in the direction for
			// which the vertices wind counter-clockwise.
			std::vector<size_t> verts;

			face(const std::vector<size_t> &_verts);

			// Returns a view that will yield neighboring vertices
			std::ranges::view auto vertices(const polyhedron &p) const &;
			std::ranges::view auto vertices(const polyhedron &p) const && = delete;
			// Returns a range that will yield neighboring edges
			std::ranges::range auto edges() const &;
			std::ranges::range auto edges() const && = delete;

			std::vector<vplane> fe_planes(const polyhedron &p) const;

			bool has_edge(const edge &e) const;
			bool has_vertex(size_t i) const;

			vec3 normal(const polyhedron &p) const;
			// Returns a copy of `e` with its two vertices arranged in CCW winding
			// order around the face
			edge get_ccw(const edge &e) const;

			friend bool operator==(const face &f1, const face &f2);
		};

		using feature = std::variant<vertex, edge, face>;

		// This can either be a V-E plane or an F-E plane. The convention here
		// is that a Voronoi plane points away from the Voronoi region it borders.
		// Note that vertices have only V-E planes, faces have only F-E planes,
		// and edges have both. For each of an edge's planes, there is a neighbor
		// with a plane at the same position, but in the opposite direction.
		struct vplane {
			feature f1;
			// TODO: Make this an edge instead of a general feature
			feature f2;
			vec3 pos{};
			vec3 dir{};

			vplane(
				const feature &_f1,
				const feature &_f2,
				const vec3 &_pos,
				const vec3 &_dir
			);

			real dist_from(const vec3 &v) const;
			const feature& other(const feature &f) const;

			friend bool operator==(const vplane &v1, const vplane &v2);
		};

		std::vector<vplane> vplanes(const polyhedron &p, const feature &f);

		enum class algorithm_step {
			Continue,
			Penetration,
			Done
		};

		struct algorithm_state {
			const polyhedron &p;
			feature f1;
			feature f2;
			algorithm_step step{ algorithm_step::Continue };

			algorithm_state(
				const polyhedron &_p,
				const feature &_f1,
				const feature &_f2
			);
		};

		struct algorithm_state_update {
			feature f1;
			feature f2;
			algorithm_step step;
			real penetration{};

			friend bool operator==(
				const algorithm_state_update &upd1,
				const algorithm_state_update &upd2
			);
		};

		algorithm_state_update vv_state(
			const polyhedron &p_v1,
			const polyhedron &p_v2,
			const vertex &v1,
			const vertex &v2
		);

		algorithm_state_update ve_state(
			const polyhedron &p_v,
			const polyhedron &p_e,
			const vertex &v,
			const edge &e
		);

		algorithm_state_update vf_state(
			const polyhedron &p_v,
			const polyhedron &p_f,
			const vertex &v,
			const face &f
		);

		algorithm_state_update ee_state(
			const polyhedron &p_e1,
			const polyhedron &p_e2,
			const edge &e1,
			const edge &e2
		);

		algorithm_state_update ef_state(
			const polyhedron &p_e,
			const polyhedron &p_f,
			const edge &e,
			const face &f
		);

		struct clip_result {
			edge e;
			feature f;
			std::optional<feature> n1{};
			std::optional<feature> n2{};
			real l1{};
			real l2{ 1 };
			bool is_clipped{};

			clip_result(const edge &_e, const feature &_f);
			clip_result(
				const edge &_e,
				const feature &_f,
				const std::optional<feature> &_n1,
				const std::optional<feature> &_n2,
				real _l1,
				real _l2,
				bool _is_partially_clipped
			);

			friend bool operator==(const clip_result &cr1, const clip_result &cr2);
		};

		// Algorithm 1 in the original paper
		clip_result clip_edge(
			const polyhedron &p_e,
			const edge &e,
			const feature &f,
			const std::vector<vplane> &vps,
			std::optional<clip_result> prev_result = {}
		);

		// Algorithm 2 in the paper
		std::optional<feature> deriv_check(
			const polyhedron &p_e,
			const polyhedron &p_f,
			const clip_result &cr
		);

		class geometry_error : public std::runtime_error {
		public:
			const feature offending_feature;

			geometry_error(const feature &_offending_feature, const std::string &message);
		};
	}
}

namespace traits {
	template <>
	std::string to_string(const phys::vclip::vertex &v, size_t indent);

	template <>
	std::string to_string(const phys::vclip::edge &e, size_t indent);

	template <>
	std::string to_string(const phys::vclip::face &f, size_t indent);

	template <>
	std::string to_string(const phys::vclip::vplane &vp, size_t indent);

	template <>
	std::string to_string(const phys::vclip::algorithm_step &step, size_t indent);

	template <>
	std::string to_string(const phys::vclip::clip_result &cr, size_t indent);

	template <>
	std::string to_string(const phys::vclip::algorithm_state_update &upd, size_t indent);
}

inline std::ranges::view auto phys::vclip::vertex::edges(const polyhedron &p) const & {
	return p.edges | std::ranges::views::filter(
		[&](const edge &e) {
			return e.has_vertex(i);
		}
	);
}

inline std::ranges::view auto phys::vclip::edge::vertices(const polyhedron &p) const & {
	return v_is | std::ranges::views::transform(
		[&](const size_t i) {
			return p.vertices[i];
		}
	);
}

inline std::ranges::view auto phys::vclip::edge::faces(const polyhedron &p) const & {
	// TODO: Cache these faces? There can only be two anyway
	return p.faces | std::ranges::views::filter(
		[&](const face &f) {
			return f.has_edge(*this);
		}
	);
}

inline std::ranges::view auto phys::vclip::face::vertices(const polyhedron &p) const & {
	return verts | std::ranges::views::transform(
		[&](const size_t i) {
			return p.vertices[i];
		}
	);
}

inline std::ranges::range auto phys::vclip::face::edges() const & {
	return util::concat_views(
		verts | std::ranges::views::slide(2) | std::ranges::views::transform(
			[&](const auto &vs) {
				return edge(vs[0], vs[1]);
			}
		),
		std::ranges::owning_view(std::vector({ edge(verts[verts.size() - 1], verts[0])}))
	);
}