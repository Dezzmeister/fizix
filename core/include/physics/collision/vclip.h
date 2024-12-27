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

		// A collection of features, defined with reference to a set of vertices.
		// A polyhedron must be convex. `validate()` will verify that the polyhedron
		// is convex and closed.
		struct polyhedron {
			std::vector<vertex> vertices{};
			std::vector<edge> edges{};
			std::vector<face> faces{};

			int euler_characteristic() const;

			void validate() const;
			void validate_references() const;
			void validate_geometry() const;

			void add_vertex(const vec3 &v);
			void add_edge(const edge &e);
			// Adds a face, but not its constituent edges. The edges must be added
			// separately with `add_edge` or the face must be added with
			// `add_face_and_new_edges`.
			void add_face(const face &f);
			// Adds a face. If any of the face's constituent edges are new, those
			// will be added as well.
			void add_face_and_new_edges(const face &f);
			std::vector<face> remove_vertex(size_t vertex_idx);
			std::vector<face> remove_edge(const edge &e);
			// Removes a face, but not its constituent edges. The edges must be removed
			// separately with `remove_edge` or the face must be removed with
			// `remove_face_and_dead_edges`.
			std::vector<face> remove_face(const face &f);
			// Removes a face. If any of the face's constituent edges would be unused after
			// removing the face, those will be removed as well.
			void remove_face_and_dead_edges(const face &f);

			bool is_possible_vertex(size_t vertex_idx) const;
			bool is_possible_edge(const edge &e) const;
			bool is_possible_face(const face &f) const;

			std::ranges::range auto features() const &;
			std::ranges::range auto features() const && = delete;

		private:
			void move_vertex(size_t from, size_t to);
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

		enum convexity {
			Convex,
			Nonconvex,
			Unspecified
		};

		struct face_cut_result;

		struct face {
			face(const std::vector<size_t> &_vs, convexity _convexity_hint = convexity::Convex);

			// Returns a view that will yield neighboring vertices
			std::ranges::view auto vertices(const polyhedron &p) const &;
			std::ranges::view auto vertices(const polyhedron &p) const && = delete;
			// Returns a range that will yield neighboring edges
			std::ranges::range auto edges() const &;
			std::ranges::range auto edges() const && = delete;

			std::vector<vplane> fe_planes(const polyhedron &p) const;

			bool has_edge(const edge &e) const;
			bool has_vertex(size_t i) const;

			// Returns the positions of the face's vertices in the polyhedron's
			// vertex array. The winding order of the vertices determines the
			// orientation of the face; the face normal points in the direction for
			// which the vertices wind counter-clockwise.
			const std::vector<size_t>& verts() const;

			inline size_t vert(size_t vert_idx) const {
				assert(vert_idx < vs.size());

				return vs[vert_idx];
			}

			inline void set_vert(size_t vert_idx, size_t new_vert) {
				assert(vert_idx < vs.size());

				vs[vert_idx] = new_vert;
				norm_needs_update = true;
			}

			size_t num_verts() const;

			// Computes the normal of the face. For a convex face, this is an O(1)
			// operation; for a nonconvex face, this is an O(N) operation where N
			// is the number of vertices.
			// The normal is defined by the winding order of the vertices so that
			// it points in the direction of whatever side has the vertices in CCW
			// winding order.
			vec3 normal(const polyhedron &p) const;
			// Returns a copy of `e` with its two vertices arranged in CCW winding
			// order around the face
			edge get_ccw(const edge &e) const;

			bool is_coplanar(const polyhedron &p) const;
			// Returns true if the vertex is convex. Uses the convexity hint to
			// short-circuit computation if possible.
			// Accepts the vertex's index in the face, not in the polyhedron.
			bool is_vertex_convex(const polyhedron &p, size_t vert_idx) const;
			// Returns true if the convexity hint is Convex and false if the hint is
			// Nonconvex. If the hint is Unspecified, the convexity is computed
			// and not cached.
			bool is_convex(const polyhedron &p) const;

			face_cut_result cut(size_t from_idx, size_t to_idx) const;
			bool can_make_cut(size_t from_idx, size_t to_idx) const;

			// Returns a copy of this face with the vertices reversed
			face flipped() const;

			friend bool operator==(const face &f1, const face &f2);

		private:
			std::vector<size_t> vs;
			mutable vec3 norm{};
			convexity convexity_hint;
			bool norm_needs_update{ true };
		};

		struct face_cut_result {
			face f1;
			face f2;
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
			feature f1;
			feature f2;
			algorithm_step step;
			real penetration{};

			friend bool operator==(
				const algorithm_state &upd1,
				const algorithm_state &upd2
			);
		};

		struct algorithm_result {
			const polyhedron &p1;
			const polyhedron &p2;
			algorithm_state state;

			algorithm_result(
				const polyhedron &_p1,
				const polyhedron &_p2,
				algorithm_state &_state
			);
		};

		algorithm_state vv_state(
			const polyhedron &p_v1,
			const polyhedron &p_v2,
			const vertex &v1,
			const vertex &v2
		);

		algorithm_state ve_state(
			const polyhedron &p_v,
			const polyhedron &p_e,
			const vertex &v,
			const edge &e
		);

		algorithm_state vf_state(
			const polyhedron &p_v,
			const polyhedron &p_f,
			const vertex &v,
			const face &f
		);

		algorithm_state ee_state(
			const polyhedron &p_e1,
			const polyhedron &p_e2,
			const edge &e1,
			const edge &e2
		);

		algorithm_state ef_state(
			const polyhedron &p_e,
			const polyhedron &p_f,
			const edge &e,
			const face &f
		);

		algorithm_result closest_features(
			const polyhedron &p1,
			const polyhedron &p2,
			const feature &_f1,
			const feature &_f2,
			size_t max_steps = SIZE_MAX
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
	std::string to_string(const phys::vclip::polyhedron &p, size_t indent);

	template <>
	std::string to_string(const phys::vclip::vplane &vp, size_t indent);

	template <>
	std::string to_string(const phys::vclip::algorithm_step &step, size_t indent);

	template <>
	std::string to_string(const phys::vclip::clip_result &cr, size_t indent);

	template <>
	std::string to_string(const phys::vclip::algorithm_state &upd, size_t indent);

	template <>
	std::string to_string(const phys::vclip::algorithm_result &result, size_t indent);
}

inline std::ranges::range auto phys::vclip::polyhedron::features() const & {
	auto to_feature =
		[&]<typename T>(const T &t) {
			return feature(t);
		};

	return util::concat_views(
		vertices | std::ranges::views::transform(to_feature),
		util::concat_views(
			edges | std::ranges::views::transform(to_feature),
			faces | std::ranges::views::transform(to_feature)
		)
	);
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
	return vs | std::ranges::views::transform(
		[&](const size_t i) {
			return p.vertices[i];
		}
	);
}

inline std::ranges::range auto phys::vclip::face::edges() const & {
	return util::concat_views(
		vs | std::ranges::views::slide(2) | std::ranges::views::transform(
			[&](const auto &vs) {
				return edge(vs[0], vs[1]);
			}
		),
		std::ranges::owning_view(std::vector({ edge(vs[vs.size() - 1], vs[0])}))
	);
}