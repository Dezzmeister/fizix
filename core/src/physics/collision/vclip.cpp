#include <algorithm>
#include "logging.h"
#include "physics/collision/vclip.h"
#include "util.h"

using namespace phys::literals;

namespace phys {
	namespace vclip {
		struct local_min_result {
			face f;
			real d{};
			algorithm_step step{ algorithm_step::Continue };
		};

		real dp(const vec3 &v, const vec3 &p_pos, const vec3 &p_dir) {
			return dot(v - p_pos, p_dir);
		}

		clip_result clip_edge(
			const polyhedron &p_e,
			const edge &e,
			const feature &f,
			const std::vector<vplane> &vps,
			std::optional<clip_result> prev_result
		) {
			clip_result out(e, f);

			if (prev_result) {
				out = *prev_result;
				out.e = e;
				out.f = f;
			}

			for (const vplane &vp : vps) {
				const feature &n = vp.other(f);
				real dt = -dp(e.t(p_e).v, vp.pos, vp.dir);
				real dh = -dp(e.h(p_e).v, vp.pos, vp.dir);

				if (dt < 0 && dh < 0) {
					out.n1 = n;
					out.n2 = n;
					out.is_clipped = false;
					return out;
				} else if (dt < 0) {
					real l = dt / (dt - dh);

					if (l > out.l1) {
						out.l1 = l;
						out.n1 = n;

						if (out.l1 > out.l2) {
							out.is_clipped = false;
							return out;
						}
					}
				} else if (dh < 0) {
					real l = dt / (dt - dh);

					if (l < out.l2) {
						out.l2 = l;
						out.n2 = n;

						if (out.l1 > out.l2) {
							out.is_clipped = false;
							return out;
						}
					}
				}
			}

			out.is_clipped = true;
			return out;
		}
		std::optional<feature> deriv_check(
			const polyhedron &p_e,
			const polyhedron &p_f,
			const clip_result &cr
		) {
			vec3 u = cr.e.t(p_e).v - cr.e.h(p_e).v;
			const feature * f = &cr.f;

			if (std::holds_alternative<edge>(*f)) {
				const edge &f_e = std::get<edge>(*f);

				if (cr.n1 && ! cr.n2) {
					f = &(*cr.n1);
				} else if (cr.n2 && ! cr.n1) {
					f = &(*cr.n2);
				} else if (cr.n1 && cr.n2) {
					real d1;
					real d2;

					if (std::holds_alternative<vertex>(*cr.n1)) {
						const vertex &v = std::get<vertex>(*cr.n1);

						d1 = dot(u, cr.e.at(p_e, cr.l1) - v.v);
					} else {
						assert(std::holds_alternative<face>(*cr.n1));

						const face &f1 = std::get<face>(*cr.n1);
						vec3 f_pos = f_e.t(p_f).v;
						d1 = dp(cr.e.at(p_e, cr.l1), f_pos, f1.normal(p_f));
					}

					if (std::holds_alternative<vertex>(*cr.n2)) {
						const vertex &v = std::get<vertex>(*cr.n2);

						d2 = dot(u, cr.e.at(p_e, cr.l2) - v.v);
					} else {
						assert(std::holds_alternative<face>(*cr.n2));

						const face &f2 = std::get<face>(*cr.n2);
						vec3 f_pos = f_e.t(p_f).v;
						d2 = dp(cr.e.at(p_e, cr.l2), f_pos, f2.normal(p_f));
					}

					if (d1 < d2) {
						f = &(*cr.n1);
					} else if (d2 < d1) {
						f = &(*cr.n2);
					} else {
						return std::nullopt;
					}
				} else {
					return std::nullopt;
				}

				assert(! std::holds_alternative<edge>(*f));
			}

			if (std::holds_alternative<vertex>(*f)) {
				const vertex &v = std::get<vertex>(*f);

				real d1 = dot(u, cr.e.at(p_e, cr.l1) - v.v);
				real d2 = dot(u, cr.e.at(p_e, cr.l2) - v.v);

				if (cr.n1 && d1 > 0) {
					return cr.n1;
				} else if (cr.n2 && d2 < 0) {
					return cr.n2;
				}

				return std::nullopt;
			}

			const face &fa = std::get<face>(*f);
			vec3 n = fa.normal(p_f);
			real un = dot(u, n);

			if (cr.n1 && un < 0) {
				return cr.n1;
			} else if (cr.n2 && un > 0) {
				return cr.n2;
			}

			return std::nullopt;
		}

		// Algorithm 3 in the paper
		local_min_result handle_local_min(
			const polyhedron &p_f,
			const face &f,
			const vertex &v
		) {
			real d_max = -infinity;
			const face * f_out = &f;

			for (const face &fp : p_f.faces) {
				vec3 n = fp.normal(p_f);
				real d = dp(v.v, p_f.vertices[fp.verts()[0]].v, n);

				if (d > d_max) {
					d_max = d;
					f_out = &fp;
				}
			}

			if (d_max <= 0) {
				return local_min_result{
					.f = *f_out,
					.d = d_max,
					.step = algorithm_step::Penetration
				};
			}

			return local_min_result{
				.f = *f_out,
				.d = d_max,
				.step = algorithm_step::Continue
			};
		}

		void polyhedron::validate() const {
			validate_references();
			validate_geometry();
		}

		void polyhedron::validate_references() const {
			size_t num_verts = vertices.size();

			for (const edge &e : edges) {
				for (size_t v_i : e.v_is) {
					if (v_i >= num_verts) {
						throw geometry_error(e,
							"Edge refers to a vertex that doesn't exist: " + traits::to_string(v_i) +
							" (there are " + traits::to_string(num_verts) + " vertices)"
						);
					}
				}
			}

			for (const face &f : faces) {
				for (size_t v_i : f.verts()) {
					if (v_i >= num_verts) {
						throw geometry_error(f,
							"Face refers to a vertex that doesn't exist: " + traits::to_string(v_i) +
							" (there are " + traits::to_string(num_verts) + " vertices)"
						);
					}
				}
			}
		}

		void polyhedron::validate_geometry() const {
			for (const face &f : faces) {
				if (f.num_verts() < 3) {
					throw geometry_error(f,
						"Face has less than three vertices: " + traits::to_string(f.num_verts())
					);
				}
			}

			for (const edge &e : edges) {
				std::vector<face> neighbors = e.faces(*this) | std::ranges::to<std::vector>();

				if (neighbors.size() != 2) {
					throw geometry_error(e,
						"Edge does not have two neighboring faces: " + traits::to_string(neighbors.size())
					);
				}

				const face &f1 = neighbors[0];
				const face &f2 = neighbors[1];
				const edge &f1e = f1.get_ccw(e);
				const vec3 bt1 = normalize(f1e.h(*this).v - f1e.t(*this).v);
				const vec3 n1 = f1.normal(*this);
				const vec3 n2 = f2.normal(*this);

				// TODO: Verify that the face normals are smooth and that the polyhedron is
				// not self-intersecting

				// Vectors tangent to the face, pointing from the edge to the other
				// side of the face
				const vec3 tan1 = cross(n1, bt1);
				const vec3 tan2 = cross(n2, -bt1);
				const vec3 avg_n = (n1 + n2) / 2.0_r;

				// For a non-convex shape that does not self-intersect, there must be at
				// least one edge whose two faces point "towards" each other
				if (dot(avg_n, tan1) > 0.0_r || dot(avg_n, tan2) > 0.0_r) {
					throw geometry_error(e, "Edge has non-convex neighbor faces");
				}
			}
		}

		size_t polyhedron::add_vertex(const vec3 &v) {
			vertices.push_back(vertex(v, vertices.size()));

			return vertices.size() - 1;
		}

		void polyhedron::add_edge(const edge &e) {
			edges.push_back(e);
		}

		void polyhedron::add_face(const face &f) {
			faces.push_back(f);
		}

		void polyhedron::add_face_and_new_edges(const face &f) {
			faces.push_back(f);

			// TODO: Sort edges, reduce time complexity from O(nm) to O(n + m)
			for (const edge &face_edge : f.edges()) {
				for (const edge &e : edges) {
					if (e == face_edge) {
						goto next_face_edge;
					}
				}

				edges.push_back(face_edge);

				next_face_edge:;
			}
		}

		std::vector<face> polyhedron::remove_vertex(size_t vertex_idx) {
			assert(is_possible_vertex(vertex_idx));

			std::vector<edge> edges_to_remove{};

			auto pred = [=](const edge &e) {
				return e.has_vertex(vertex_idx);
			};
			auto it = std::begin(edges);
			while ((it = std::find_if(it, std::end(edges), pred)) != std::end(edges)) {
				edges_to_remove.push_back(*it);
				++it;
			}

			std::vector<face> out{};

			for (const edge &e : edges_to_remove) {
				std::vector<face> removed_faces = remove_edge(e);

				std::move(std::begin(removed_faces), std::end(removed_faces), std::back_inserter(out));
			}

			for (size_t i = vertex_idx + 1; i < vertices.size(); i++) {
				move_vertex(i, i - 1);
			}

			std::erase(vertices, vertices[vertex_idx]);

			return out;
		}

		std::vector<face> polyhedron::remove_edge(const edge &e) {
			assert(is_possible_edge(e));

			auto it = std::partition(std::begin(faces), std::end(faces), [&](const face &f) {
				return ! f.has_edge(e);
			});

			std::vector<face> out{};
			std::move(it, std::end(faces), std::back_inserter(out));
			faces.erase(it, std::end(faces));
			std::erase(edges, e);

			return out;
		}

		std::vector<face> polyhedron::remove_face(const face &f) {
			assert(is_possible_face(f));

			if (std::erase(faces, f)) {
				return { f };
			}

			return {};
		}

		void polyhedron::remove_face_and_dead_edges(const face &f) {
			assert(is_possible_face(f));

			face old_f = f;
			std::erase(faces, f);

			// TODO: reduce time complexity
			for (const edge &face_edge : old_f.edges()) {
				if (face_edge.faces(*this).empty()) {
					std::erase(edges, face_edge);
				}
			}
		}

		bool polyhedron::is_possible_vertex(size_t vertex_idx) const {
			return vertex_idx < vertices.size();
		}

		bool polyhedron::is_possible_edge(const edge &e) const {
			return is_possible_vertex(e.v_is[0]) && is_possible_vertex(e.v_is[1]);
		}

		bool polyhedron::is_possible_face(const face &f) const {
			for (size_t v_idx : f.verts()) {
				if (! is_possible_vertex(v_idx)) {
					return false;
				}
			}

			return true;
		}

		bool polyhedron::has_edge(const edge &e) const {
			for (const edge &pe : edges) {
				if (pe == e) {
					return true;
				}
			}

			return false;
		}

		bool polyhedron::has_face(const face &f) const {
			for (const face &pf : faces) {
				if (pf == f) {
					return true;
				}
			}

			return false;
		}

		std::optional<face> polyhedron::superset_face(const face &f) const {
			assert(f.num_verts() >= 3);

			for (size_t i = 0; i < faces.size(); i++) {
				const face &pf = faces[i];
				int64_t pf_start = -1;

				for (size_t j = 0; j < pf.num_verts(); j++) {
					if (pf.vert(j) == f.vert(0)) {
						pf_start = j + 1;
						break;
					}
				}

				if (pf_start == -1) {
					continue;
				}

				size_t fi = 1;

				for (size_t j = (size_t)pf_start; j < pf.num_verts(); j++) {
					if (fi >= f.num_verts()) {
						return pf;
					}

					if (pf.vert(j) == f.vert(fi)) {
						fi++;
					}
				}

				for (size_t j = 0; j < (size_t)pf_start; j++) {
					if (fi >= f.num_verts()) {
						return pf;
					}

					if (pf.vert(j) == f.vert(fi)) {
						fi++;
					}
				}

				if (fi >= f.num_verts()) {
					return pf;
				}
			}

			return std::nullopt;
		}

		void polyhedron::clear() {
			faces.clear();
			edges.clear();
			vertices.clear();
		}

		void polyhedron::move_vertex(size_t from, size_t to) {
			for (edge &e : edges) {
				if (e.v_is[0] == from) {
					e.v_is[0] = to;
					assert(e.v_is[1] != from);
				} else if (e.v_is[1] == from) {
					e.v_is[1] = to;
				}
			}

			for (face &f : faces) {
				for (size_t i = 0; i < f.num_verts(); i++) {
					if (f.vert(i) == from) {
						f.set_vert(i, to);
					}
				}
			}
		}

		int polyhedron::euler_characteristic() const {
			return (int)(vertices.size() + faces.size()) - (int)edges.size();
		}

		vertex::vertex(const vec3 &_v, size_t _i) :
			v(_v), i(_i) {}

		edge::edge(size_t _t_i, size_t _h_i) :
			v_is({ _t_i, _h_i }) {}

		const vertex& edge::t(const polyhedron &p) const {
			return p.vertices[v_is[0]];
		}

		const vertex& edge::h(const polyhedron &p) const {
			return p.vertices[v_is[1]];
		}

		std::vector<vplane> vertex::ve_planes(const polyhedron &p) const {
			std::vector<vplane> out{};

			for (const edge &e : edges(p)) {
				vec3 norm;

				if (e.t(p) == *this) {
					norm = e.h(p).v - e.t(p).v;
				} else {
					norm = e.t(p).v - e.h(p).v;
				}

				out.push_back(vplane(
					*this,
					e,
					v,
					normalize(norm)
				));
			}

			return out;
		}

		std::vector<vplane> edge::ve_planes(const polyhedron &p) const {
			std::vector<vplane> out{};
			const vertex &v1 = p.vertices[v_is[0]];
			const vertex &v2 = p.vertices[v_is[1]];
			vec3 normal = normalize(v1.v - v2.v);

			out.push_back(vplane(
				v1,
				*this,
				v1.v,
				normal
			));
			out.push_back(vplane(
				v2,
				*this,
				v2.v,
				-normal
			));

			return out;
		}

		std::vector<vplane> edge::fe_planes(const polyhedron &p) const {
			std::vector<vplane> out{};
			auto fs = faces(p);
			auto it = std::begin(fs);

			assert(it != std::end(fs));
			face f1 = *it;
			edge e1 = f1.get_ccw(*this);
			vec3 e1_d = normalize(e1.h(p).v - e1.t(p).v);
			++it;

			assert(it != std::end(fs));
			face f2 = *it;
			edge e2 = f2.get_ccw(*this);
			vec3 e2_d = normalize(e2.h(p).v - e2.t(p).v);
			++it;

			assert(it == std::end(fs));

			out.push_back(vplane(
				f1,
				e1,
				e1.t(p).v,
				cross(f1.normal(p), e1_d)
			));
			out.push_back(vplane(
				f2,
				e2,
				e2.t(p).v,
				cross(f2.normal(p), e2_d)
			));

			return out;
		}

		bool edge::has_vertex(size_t i) const {
			return v_is[0] == i || v_is[1] == i;
		}

		vec3 edge::at(const polyhedron &p, real l) const {
			assert(l >= 0);
			assert(l <= 1);

			return (1 - l) * t(p).v + l * h(p).v;
		}

		bool is_colinear(const polyhedron &p, const edge &e1, const edge &e2) {
			vec3 e1v = e1.t(p).v - e1.h(p).v;
			vec3 e2v = e2.t(p).v - e2.h(p).v;

			// TODO: Profile & optimize
			return cross(e1v, e2v) == vec3(0.0_r);
		}

		face::face(const std::vector<size_t> &_vs, convexity _convexity_hint) : 
			vs(_vs), convexity_hint(_convexity_hint) {}

		std::vector<vplane> face::fe_planes(const polyhedron &p) const {
			std::vector<vplane> out{};
			vec3 n = normal(p);

			for (const edge &e : edges()) {
				vec3 d = normalize(e.h(p).v - e.t(p).v);

				out.push_back(vplane(
					*this,
					e,
					e.t(p).v,
					cross(d, n)
				));
			}

			return out;
		}

		bool face::has_edge(const edge &e) const {
			auto all_edges = edges();

			return std::find(std::begin(all_edges), std::end(all_edges), e) != std::end(all_edges);
		}

		bool face::has_vertex(size_t i) const {
			return std::find(std::begin(vs), std::end(vs), i) != std::end(vs);
		}

		const std::vector<size_t>& face::verts() const {
			return vs;
		}

		size_t face::num_verts() const {
			return vs.size();
		}

		vec3 face::normal(const polyhedron &p, bool force_recompute) const {
			if (! norm_needs_update && ! force_recompute) {
				return norm;
			}

			norm_needs_update = false;

			if (convexity_hint == convexity::Convex) {
				vec3 v1 = p.vertices[vs[0]].v;
				vec3 v2 = p.vertices[vs[1]].v;
				vec3 v3 = p.vertices[vs[2]].v;
				vec3 e1 = v2 - v1;
				vec3 e2 = v3 - v2;

				norm = normalize(cross(e1, e2));
				return norm;
			}

			// We can't just take the cross product of the first two edges, because
			// the direction of the normal will be flipped if the edges are concave.
			// Instead, we can compute the bounding box of the polygon, then find a
			// vertex along the bounding box and use that to compute the normal.
			// A vertex along the bounding box must be convex, so the normal will point
			// in the correct direction.
			// This won't work if the bounding box is degenerate in any direction, so
			// we set degenerate planes to infinity or negative infinity before finding
			// a convex vertex.

			assert(is_coplanar(p));

			vec3 min(infinity);
			vec3 max(-infinity);

			for (const vertex &v : vertices(p)) {
				if (v.v.x < min.x) {
					min.x = v.v.x;
				}
				if (v.v.y < min.y) {
					min.y = v.v.y;
				}
				if (v.v.z < min.z) {
					min.z = v.v.z;
				}
				if (v.v.x > max.x) {
					max.x = v.v.x;
				}
				if (v.v.y > max.y) {
					max.y = v.v.y;
				}
				if (v.v.z > max.z) {
					max.z = v.v.z;
				}
			}

			assert(min.x != infinity);
			assert(min.y != infinity);
			assert(min.z != infinity);
			assert(max.x != -infinity);
			assert(max.y != -infinity);
			assert(max.z != -infinity);

			if (min.x == max.x) {
				min.x = -infinity;
				max.x = infinity;
			}
			if (min.y == max.y) {
				min.y = -infinity;
				max.y = infinity;
			}
			if (min.z == max.z) {
				min.z = -infinity;
				max.z = infinity;
			}

			assert(min.x != max.x);
			assert(min.y != max.y);
			assert(min.z != max.z);

			int64_t v_idx = -1;

			for (size_t i = 0; i < vs.size(); i++) {
				const vertex &v = p.vertices[vs[i]];

				bool is_on_any_min_plane =
					(v.v.x == min.x) ||
					(v.v.y == min.y) ||
					(v.v.z == min.z);

				if (is_on_any_min_plane) {
					v_idx = i;
					break;
				}

				bool is_on_any_max_plane =
					(v.v.x == max.x) ||
					(v.v.y == max.y) ||
					(v.v.z == max.z);

				if (is_on_any_max_plane) {
					v_idx = i;
					break;
				}
			}

			assert(v_idx != -1);

			size_t vi0;
			size_t vi1 = (size_t)v_idx;
			size_t vi2;

			if (vi1 == 0) {
				vi0 = vs.size() - 1;
				vi2 = 1;
			} else if (vi1 == vs.size() - 1) {
				vi0 = vs.size() - 2;
				vi2 = 0;
			} else {
				vi0 = vi1 - 1;
				vi2 = vi1 + 1;
			}

			vec3 v1 = p.vertices[vs[vi0]].v;
			vec3 v2 = p.vertices[vs[vi1]].v;
			vec3 v3 = p.vertices[vs[vi2]].v;
			vec3 e1 = v2 - v1;
			vec3 e2 = v3 - v2;

			norm = normalize(cross(e1, e2));
			return norm;
		}

		edge face::get_ccw(const edge &_e) const {
			for (const edge e : edges()) {
				if (e == _e) {
					return e;
				}
			}

			throw geometry_error(*this, "Edge not found: " + traits::to_string(_e));
		}

		bool face::is_coplanar(const polyhedron &p) const {
			auto all_edges = edges();
			auto it = std::begin(all_edges);
			const edge e1 = *it;
			++it;
			assert(it != std::end(all_edges));

			const edge e2 = *it;
			++it;
			assert(it != std::end(all_edges));

			vec3 dir = cross(e1.h(p).v - e1.t(p).v, e2.h(p).v - e2.t(p).v);

			for (it; it != std::end(all_edges); it++) {
				if (std::abs(dot(dir, it->h(p).v - it->t(p).v)) > 1e-6f) {
					return false;
				}
			}

			return true;
		}

		bool face::is_vertex_convex(const polyhedron &p, size_t vert_idx) const {
			if (vert_idx >= vs.size()) {
				throw geometry_error(
					*this,
					"Face does not have vertex at index " + traits::to_string(vert_idx) +
					"; face only has " + traits::to_string(vs.size()) + " vertices"
				);
			}

			if (vs.size() < 3) {
				throw geometry_error(*this, "Face has less than three vertices");
			}

			size_t vi0;
			size_t vi1 = vert_idx;
			size_t vi2;

			if (vi1 == 0) {
				vi0 = vs.size() - 1;
				vi2 = 1;
			} else if (vi1 == vs.size() - 1) {
				vi0 = vs.size() - 2;
				vi2 = 0;
			} else {
				vi0 = vi1 - 1;
				vi2 = vi1 + 1;
			}

			vec3 face_norm = normal(p);
			vec3 v1 = p.vertices[vs[vi0]].v;
			vec3 v2 = p.vertices[vs[vi1]].v;
			vec3 v3 = p.vertices[vs[vi2]].v;
			vec3 e1 = v2 - v1;
			vec3 e2 = v3 - v2;

			vec3 local_norm = normalize(cross(e1, e2));

			return dot(face_norm, local_norm) > 0;
		}

		bool face::is_convex(const polyhedron &p) const {
			if (convexity_hint == convexity::Convex) {
				return true;
			} else if (convexity_hint == convexity::Nonconvex) {
				return false;
			}

			if (vs.size() == 3) {
				return true;
			}

			size_t vi0 = vs.size() - 1;
			size_t vi1 = 0;
			size_t vi2 = 1;
			vec3 e1 = p.vertices[vs[vi1]].v - p.vertices[vs[vi0]].v;
			vec3 e2 = p.vertices[vs[vi2]].v - p.vertices[vs[vi1]].v;
			vec3 dir = cross(e1, e2);

			for (vi1 = 1; vi1 < vs.size() - 1; vi1++) {
				vi0 = vi1 - 1;
				vi2 = vi1 + 1;
				e1 = p.vertices[vs[vi1]].v - p.vertices[vs[vi0]].v;
				e2 = p.vertices[vs[vi2]].v - p.vertices[vs[vi1]].v;

				vec3 curr_dir = cross(e1, e2);

				if (dot(dir, curr_dir) <= 0) {
					return false;
				}
			}

			vi0 = vs.size() - 2;
			vi1 = vs.size() - 1;
			vi2 = 0;
			e1 = p.vertices[vs[vi1]].v - p.vertices[vs[vi0]].v;
			e2 = p.vertices[vs[vi2]].v - p.vertices[vs[vi1]].v;

			vec3 curr_dir = cross(e1, e2);

			if (dot(dir, curr_dir) <= 0) {
				return false;
			}

			return true;
		}

		face_cut_result face::cut(size_t from_idx, size_t to_idx) const {
			assert(to_idx < vs.size());
			assert(from_idx < vs.size());

			std::vector<size_t> f1_verts{};
			std::vector<size_t> f2_verts{};

			size_t start = from_idx < to_idx ? from_idx : to_idx;
			size_t end = from_idx < to_idx ? to_idx : from_idx;

			for (size_t i = start; i <= end; i++) {
				f1_verts.push_back(vs[i]);
			}

			for (size_t i = end; i < vs.size(); i++) {
				f2_verts.push_back(vs[i]);
			}

			for (size_t i = 0; i <= start; i++) {
				f2_verts.push_back(vs[i]);
			}

			if (f1_verts.size() < 3 || f2_verts.size() < 3) {
				throw geometry_error(
					*this,
					"Invalid cut from vertex " + traits::to_string(from_idx) +
					" to " + traits::to_string(to_idx)
				);
			}

			convexity new_hint = convexity_hint == convexity::Convex ?
				convexity::Convex : convexity::Unspecified;

			return face_cut_result{
				.f1 = face(f1_verts, new_hint),
				.f2 = face(f2_verts, new_hint)
			};
		}

		bool face::can_make_cut(size_t from_idx, size_t to_idx) const {
			assert(from_idx < vs.size());
			assert(to_idx < vs.size());

			size_t start = from_idx < to_idx ? from_idx : to_idx;
			size_t end = from_idx < to_idx ? to_idx : from_idx;

			if (end - start < 2) {
				return false;
			}

			return ! (start == 0 && end == vs.size() - 1);
		}

		void face::fix_colinear_edges(const polyhedron &p) {
			assert(vs.size() >= 3);
			bool had_colinear_edges = false;

			for (size_t i = vs.size() - 2; i >= 1; i--) {
				edge e1(vs[i - 1], vs[i]);
				edge e2(vs[i], vs[i + 1]);

				if (is_colinear(p, e1, e2)) {
					had_colinear_edges = true;
					vs.erase(std::begin(vs) + i);
				}
			}

			edge e1(vs[vs.size() - 1], vs[0]);
			edge e2(vs[0], vs[1]);

			if (is_colinear(p, e1, e2)) {
				had_colinear_edges = true;
				vs.erase(std::begin(vs));
			}

			e1 = edge(vs[vs.size() - 2], vs[vs.size() - 1]);
			e2 = edge(vs[vs.size() - 1], vs[0]);

			if (is_colinear(p, e1, e2)) {
				had_colinear_edges = true;
				vs.erase(std::begin(vs) + vs.size() - 1);
			}

			if (had_colinear_edges) {
				fix_colinear_edges(p);
			}
		}

		face face::flipped() const {
			return face(
				std::ranges::views::reverse(vs) | std::ranges::to<std::vector<size_t>>(),
				convexity_hint
			);
		}

		vplane::vplane(
			const feature &_f1,
			const feature &_f2,
			const vec3 &_pos,
			const vec3 &_dir
		) :
			f1(_f1),
			f2(_f2),
			pos(_pos),
			dir(_dir)
		{}

		const feature& vplane::other(const feature &f) const {
			if (f == f1) {
				return f2;
			} else if (f == f2) {
				return f1;
			}

			std::unreachable();
		}

		std::vector<vplane> vplanes(const polyhedron &p, const feature &_f) {
			if (std::holds_alternative<vertex>(_f)) {
				const vertex &v = std::get<vertex>(_f);

				return v.ve_planes(p);
			} else if (std::holds_alternative<face>(_f)) {
				const face &f = std::get<face>(_f);

				return f.fe_planes(p);
			}

			const edge &e = std::get<edge>(_f);
			std::vector<vplane> out = e.ve_planes(p);
			std::vector<vplane> fe_planes = e.fe_planes(p);

			out.insert(std::end(out), std::begin(fe_planes), std::end(fe_planes));

			return out;
		}

		algorithm_state vv_state(
			const polyhedron &p1,
			const polyhedron &p2,
			const vertex &v1,
			const vertex &v2
		) {
			std::vector<vplane> vps = v1.ve_planes(p1); 

			for (const auto &vp : vps) {
				vec3 v_dir = v2.v - vp.pos;

				if (dot(v_dir, vp.dir) >= 0) {
					return algorithm_state{
						.f1 = vp.f2,
						.f2 = v2,
						.step = algorithm_step::Continue
					};
				}
			}

			vps = v2.ve_planes(p2);

			for (const auto &vp : vps) {
				vec3 v_dir = v1.v - vp.pos;

				if (dot(v_dir, vp.dir) >= 0) {
					return algorithm_state{
						.f1 = v1,
						.f2 = vp.f2,
						.step = algorithm_step::Continue
					};
				}
			}

			return algorithm_state{
				.f1 = v1,
				.f2 = v2,
				.step = algorithm_step::Done
			};
		}

		algorithm_state ve_state(
			const polyhedron &p_v,
			const polyhedron &p_e,
			const vertex &v,
			const edge &e
		) {
			std::vector<vplane> ve_vps = e.ve_planes(p_e);
			for (const auto &vp : ve_vps) {
				vec3 v_dir = v.v - vp.pos;

				if (dot(v_dir, vp.dir) > 0) {
					return algorithm_state{
						.f1 = v,
						.f2 = vp.f1,
						.step = algorithm_step::Continue
					};
				}
			}

			std::vector<vplane> fe_vps = e.fe_planes(p_e);
			for (const auto &vp : fe_vps) {
				vec3 v_dir = v.v - vp.pos;

				if (dot(v_dir, vp.dir) > 0) {
					return algorithm_state{
						.f1 = v,
						.f2 = vp.f1,
						.step = algorithm_step::Continue
					};
				}
			}

			clip_result cr = clip_edge(p_e, e, v, vplanes(p_v, v));
			algorithm_state out{
				.f1 = v,
				.f2 = e,
				.step = algorithm_step::Continue
			};
			bool was_updated = false;

			if (cr.n1 == cr.n2 && !! cr.n1) {
				out.f1 = *cr.n2;
				was_updated = true;
			} else {
				std::optional<feature> f_opt = deriv_check(p_e, p_v, cr);

				if (f_opt) {
					out.f1 = *f_opt;
					was_updated = true;
				}
			}

			if (! was_updated) {
				out.step = algorithm_step::Done;
			}

			return out;
		}

		algorithm_state vf_state(
			const polyhedron &p_v,
			const polyhedron &p_f,
			const vertex &v,
			const face &f
		) {
			std::vector<vplane> vps = f.fe_planes(p_f);
			real violation = 0;
			std::optional<edge> max_edge{};

			for (const vplane &vp : vps) {
				real vp_violation = dp(v.v, vp.pos, vp.dir);

				if (vp_violation > violation) {
					violation = vp_violation;
					max_edge = std::get<edge>(vp.f2);
				}
			}

			if (max_edge) {
				return algorithm_state{
					.f1 = v,
					.f2 = *max_edge,
					.step = algorithm_step::Continue
				};
			}

			vec3 f_norm = f.normal(p_f);
			real dpv = dp(v.v, p_f.vertices[f.vert(0)].v, f_norm);

			for (const edge &e : v.edges(p_v)) {
				vec3 vp = e.v_is[0] == v.i ? e.h(p_v).v : e.t(p_v).v;
				real dpvp = dp(vp, p_f.vertices[f.vert(0)].v, f_norm);

				// The algorithm as described in the paper compares the magnitudes of Dp(v)
				// and Dp(vp) here, but that would be incorrect if this vertex does not
				// penetrate the face, but the other vertex penetrates the face and
				// is further from the face than this vertex. In that case, we might
				// not detect the penetration and falsely terminate.
				bool should_update_to_vp = std::signbit(dpv) == std::signbit(dpvp) ?
					std::abs(dpv) > std::abs(dpvp) : dpv > dpvp;
				if (should_update_to_vp) {
					return algorithm_state{
						.f1 = e,
						.f2 = f,
						.step = algorithm_step::Continue
					};
				}
			}

			if (dpv > 0) {
				return algorithm_state{
					.f1 = v,
					.f2 = f,
					.step = algorithm_step::Done
				};
			}

			local_min_result lmr = handle_local_min(p_f, f, v);
			// TODO: Rename this to "distance" and always report it
			real penetration =
				lmr.step == algorithm_step::Penetration ? lmr.d : 0.0_r;

			return algorithm_state{
				.f1 = v,
				.f2 = lmr.f,
				.step = lmr.step,
				.penetration = penetration
			};
		}

		algorithm_state ee_state(
			const polyhedron &p_e1,
			const polyhedron &p_e2,
			const edge &e1,
			const edge &e2
		) {
			algorithm_state out{
				.f1 = e1,
				.f2 = e2,
				.step = algorithm_step::Continue
			};

			std::vector<vplane> vps = e1.ve_planes(p_e1);
			clip_result cr = clip_edge(p_e2, e2, e1, vps);

			if (! cr.is_clipped && cr.n1 == cr.n2 && cr.n1) {
				out.f1 = *cr.n1;
			} else {
				std::optional<feature> f_opt = deriv_check(p_e2, p_e1, cr);

				if (f_opt) {
					out.f1 = *f_opt;
				}
			}

			if (out.f1 != feature(e1)) {
				return out;
			}

			vps = e1.fe_planes(p_e1);
			cr = clip_edge(p_e2, e2, e1, vps, cr);

			if (! cr.is_clipped && cr.n1 == cr.n2 && cr.n1) {
				out.f1 = *cr.n1;
			} else {
				std::optional<feature> f_opt = deriv_check(p_e2, p_e1, cr);

				if (f_opt) {
					out.f1 = *f_opt;
				}
			}

			if (out.f1 != feature(e1)) {
				return out;
			}

			vps = e2.ve_planes(p_e2);
			cr = clip_edge(p_e1, e1, e2, vps);

			if (! cr.is_clipped && cr.n1 == cr.n2 && cr.n1) {
				out.f2 = *cr.n1;
			} else {
				std::optional<feature> f_opt = deriv_check(p_e1, p_e2, cr);

				if (f_opt) {
					out.f2 = *f_opt;
				}
			}

			if (out.f2 != feature(e2)) {
				return out;
			}

			vps = e2.fe_planes(p_e2);
			cr = clip_edge(p_e1, e1, e2, vps, cr);

			if (! cr.is_clipped && cr.n1 == cr.n2 && cr.n1) {
				out.f2 = *cr.n1;
			} else {
				std::optional<feature> f_opt = deriv_check(p_e1, p_e2, cr);

				if (f_opt) {
					out.f2 = *f_opt;
				}
			}

			if (out.f2 != feature(e2)) {
				return out;
			}

			out.step = algorithm_step::Done;
			return out;
		}

		algorithm_state ef_state(
			const polyhedron &p_e,
			const polyhedron &p_f,
			const edge &e,
			const face &f
		) {
			std::vector<vplane> vps = f.fe_planes(p_f);
			clip_result cr = clip_edge(p_e, e, f, vps);

			if (! cr.is_clipped) {
				std::optional<feature> min_f = cr.n1;

				assert(min_f);

				vps = std::get<edge>(*min_f).ve_planes(p_f);
				cr = clip_edge(p_e, e, *min_f, vps);
				std::optional<feature> next_f = deriv_check(p_e, p_f, cr);

				while (next_f) {
					assert(std::holds_alternative<vertex>(*next_f));

					min_f = next_f;
					std::vector<vplane> all_vps = std::get<vertex>(*min_f).ve_planes(p_f);

					vps.clear();

					for (const vplane &vp : all_vps) {
						assert(std::holds_alternative<edge>(vp.f2));

						if (f.has_edge(std::get<edge>(vp.f2))) {
							vps.push_back(vp);
						}
					}

					assert(! vps.empty());

					cr = clip_edge(p_e, e, *min_f, vps);
					next_f = deriv_check(p_e, p_f, cr);
				}

				return algorithm_state{
					.f1 = e,
					.f2 = *min_f,
					.step = algorithm_step::Continue
				};
			}

			vec3 n = f.normal(p_f);
			vec3 e_v1 = e.at(p_e, cr.l1);
			vec3 e_v2 = e.at(p_e, cr.l2);
			real d1 = dot(e_v1 - p_f.vertices[f.vert(0)].v, n);
			real d2 = dot(e_v2 - p_f.vertices[f.vert(0)].v, n);

			if (d1 * d2 <= 0) {
				return algorithm_state{
					.f1 = e,
					.f2 = f,
					.step = algorithm_step::Penetration,
					.penetration = d1 < 0 ? d1 : d2
				};
			}

			vec3 u = e.t(p_e).v - e.h(p_e).v;
			real un = dot(u, n);
			bool below_support = d1 < 0 && d2 < 0;

			if ((un < 0) != below_support) {
				if (cr.n1) {
					return algorithm_state{
						.f1 = e,
						.f2 = *cr.n1,
						.step = algorithm_step::Continue
					};
				} else {
					return algorithm_state{
						.f1 = e.t(p_e),
						.f2 = f,
						.step = algorithm_step::Continue
					};
				}
			} else {
				if (cr.n2) {
					return algorithm_state{
						.f1 = e,
						.f2 = *cr.n2,
						.step = algorithm_step::Continue
					};
				} else {
					return algorithm_state{
						.f1 = e.h(p_e),
						.f2 = f,
						.step = algorithm_step::Continue
					};
				}
			}
		}

		clip_result::clip_result(const edge &_e, const feature &_f) :
			e(_e), f(_f) {}

		clip_result::clip_result(
			const edge &_e,
			const feature &_f,
			const std::optional<feature> &_n1,
			const std::optional<feature> &_n2,
			real _l1,
			real _l2,
			bool _is_partially_clipped
		) :
			e(_e),
			f(_f),
			n1(_n1),
			n2(_n2),
			l1(_l1),
			l2(_l2),
			is_clipped(_is_partially_clipped)
		{}

		geometry_error::geometry_error(
			const feature &_offending_feature,
			const std::string &message
		) :
			std::runtime_error(message),
			offending_feature(_offending_feature)
		{}

		algorithm_result::algorithm_result(
			const polyhedron &_p1,
			const polyhedron &_p2,
			algorithm_state &_state
		) :
			p1(_p1),
			p2(_p2),
			state(_state)
		{}

		algorithm_state update_state(
			const polyhedron &p1,
			const polyhedron &p2,
			const algorithm_state &old
		) {
			if (std::holds_alternative<vertex>(old.f1)) {
				const vertex &v1 = std::get<vertex>(old.f1);

				if (std::holds_alternative<vertex>(old.f2)) {
					return vv_state(p1, p2, v1, std::get<vertex>(old.f2));
				} else if (std::holds_alternative<edge>(old.f2)) {
					return ve_state(p1, p2, v1, std::get<edge>(old.f2));
				} else {
					return vf_state(p1, p2, v1, std::get<face>(old.f2));
				}
			} else if (std::holds_alternative<edge>(old.f1)) {
				const edge &e1 = std::get<edge>(old.f1);

				if (std::holds_alternative<vertex>(old.f2)) {
					algorithm_state state = ve_state(p2, p1, std::get<vertex>(old.f2), e1);
					std::swap(state.f1, state.f2);

					return state;
				} else if (std::holds_alternative<edge>(old.f2)) {
					return ee_state(p1, p2, e1, std::get<edge>(old.f2));
				} else {
					return ef_state(p1, p2, e1, std::get<face>(old.f2));
				}
			} else {
				const face &f1 = std::get<face>(old.f1);

				if (std::holds_alternative<vertex>(old.f2)) {
					algorithm_state state = vf_state(p2, p1, std::get<vertex>(old.f2), f1);
					std::swap(state.f1, state.f2);

					return state;
				} else if (std::holds_alternative<edge>(old.f2)) {
					algorithm_state state = ef_state(p2, p1, std::get<edge>(old.f2), f1);
					std::swap(state.f1, state.f2);

					return state;
				} else {
					std::unreachable();
				}
			}
		}

		algorithm_result closest_features(
			const polyhedron &p1,
			const polyhedron &p2,
			const feature &_f1,
			const feature &_f2,
			size_t max_steps
		) {
			algorithm_state state = algorithm_state{
				.f1 = _f1,
				.f2 = _f2,
				.step = algorithm_step::Continue,
				.penetration = 0.0_r
			};
			size_t steps = 0;

			while (state.step == algorithm_step::Continue) {
				if (steps >= max_steps) {
					break;
				}

				state = update_state(p1, p2, state);
				steps++;
			}

			return algorithm_result(p1, p2, state);
		}

		bool operator==(const vertex &v1, const vertex &v2) {
			return v1.i == v2.i;
		}

		bool operator==(const edge &e1, const edge &e2) {
			return e1.v_is == e2.v_is || (
				e1.v_is[0] == e2.v_is[1] &&
				e1.v_is[1] == e2.v_is[0]
			);
		}

		bool operator==(const face &f1, const face &f2) {
			if (f1.vs.size() != f2.vs.size()) {
				return false;
			}

			size_t i1 = 0;
			size_t i2 = 0;

			for (; i2 < f2.vs.size(); i2++) {
				if (f1.vs[i1] == f2.vs[i2]) {
					break;
				}
			}

			if (i2 == f2.vs.size()) {
				return false;
			}

			for (size_t i = 0; i < f2.vs.size(); i++) {
				size_t j = i + i2;

				if (j >= f2.vs.size()) {
					j -= f2.vs.size();
				}

				if (f1.vs[i] != f2.vs[j]) {
					return false;
				}
			}

			return true;
		}

		bool operator==(const vplane &v1, const vplane &v2) {
			return std::tie(v1.f1, v1.f2, v1.pos, v1.dir) == std::tie(v2.f1, v2.f2, v2.pos, v2.dir);
		}

		bool operator==(const clip_result &cr1, const clip_result &cr2) {
			bool are_non_numerics_eq = std::tie(cr1.e, cr1.f, cr1.n1, cr1.n2, cr1.is_clipped) ==
				std::tie(cr2.e, cr2.f, cr2.n1, cr2.n2, cr2.is_clipped);
			bool are_numerics_eq =
				util::eq_within_epsilon(cr1.l1, cr2.l1) && util::eq_within_epsilon(cr1.l2, cr2.l2);

			return are_non_numerics_eq && are_numerics_eq;
		}

		bool operator==(const algorithm_state &upd1, const algorithm_state &upd2) {
			return std::tie(upd1.f1, upd1.f2, upd1.step) ==
				std::tie(upd2.f1, upd2.f2, upd2.step) &&
				util::eq_within_epsilon(upd1.penetration, upd2.penetration);
		}
	}
}

template <>
std::string traits::to_string(const phys::vclip::vertex &v, size_t) {
	return "vertex(" + traits::to_string(v.i) + ", " + traits::to_string(v.v) + ")";
}


template <>
std::string traits::to_string(const phys::vclip::edge &e, size_t) {
	return "edge(" + traits::to_string(e.v_is[0]) + ", " + traits::to_string(e.v_is[1]) + ")";
}


template <>
std::string traits::to_string(const phys::vclip::face &f, size_t) {
	std::stringstream out{};
	out << "face(";

	for (size_t i = 0; i < f.num_verts(); i++) {
		out << traits::to_string(f.vert(i));

		if (i + 1 != f.num_verts()) {
			out << ", ";
		}
	}

	out << ")";

	return out.str();
}

template <>
std::string traits::to_string(const phys::vclip::polyhedron &p, size_t) {
	return to_string(&p);
}

template <>
std::string traits::to_string(const phys::vclip::vplane &vp, size_t indent) {
	return util::obj_to_string(
		"vplane", indent,
		util::named_val("f1", vp.f1),
		util::named_val("f2", vp.f2),
		util::named_val("pos", vp.pos),
		util::named_val("dir", vp.dir)
	);
}

template <>
std::string traits::to_string(const phys::vclip::algorithm_step &step, size_t) {
	switch (step) {
		case phys::vclip::algorithm_step::Continue:
			return "algorithm_step::Continue";
		case phys::vclip::algorithm_step::Done:
			return "algorithm_step::Done";
		case phys::vclip::algorithm_step::Penetration:
			return "algorithm_step::Penetration";
		default:
			std::unreachable();
	}
}

template <>
std::string traits::to_string(const phys::vclip::clip_result &cr, size_t indent) {
	return util::obj_to_string(
		"clip_result", indent,
		util::named_val("e", cr.e),
		util::named_val("f", cr.f),
		util::named_val("n1", cr.n1),
		util::named_val("n2", cr.n2),
		util::named_val("l1", cr.l1),
		util::named_val("l2", cr.l2),
		util::named_val("is_clipped", cr.is_clipped)
	);
}

template <>
std::string traits::to_string(const phys::vclip::algorithm_state &upd, size_t indent) {
	return util::obj_to_string(
		"algorithm_state", indent,
		util::named_val("f1", upd.f1),
		util::named_val("f2", upd.f2),
		util::named_val("step", upd.step),
		util::named_val("penetration", upd.penetration)
	);
}

template <>
std::string traits::to_string(const phys::vclip::algorithm_result &result, size_t indent) {
	return util::obj_to_string(
		"algorithm_result", indent,
		util::named_val("p1", result.p1),
		util::named_val("p2", result.p2),
		util::named_val("state", result.state)
	);
}