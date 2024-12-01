#include <algorithm>
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
			}

			for (const vplane &vp : vps) {
				const feature &n = vp.other(f);
				real dt = -vp.dist_from(e.t(p_e).v);
				real dh = -vp.dist_from(e.h(p_e).v);

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

		real edge_dist_deriv(
			const polyhedron &p_e,
			const polyhedron &p_f,
			const clip_result &cr,
			const real l
		) {
			vec3 u = cr.e.t(p_e).v - cr.e.h(p_e).v;
			const feature * f = &cr.f;

			if (std::holds_alternative<edge>(*f)) {
				real dl = l - cr.l1;
				real dr = cr.l2 - l;
				const std::optional<feature> &f_opt = dl < dr ? cr.n1 : cr.n2;

				assert(!! f_opt);

				f = &(*f_opt);

				assert(! std::holds_alternative<edge>(*f));
			}

			if (std::holds_alternative<vertex>(*f)) {
				const vertex &v = std::get<vertex>(*f);

				return dot(u, cr.e.at(p_e, l) - v.v);
			}

			const face &fa = std::get<face>(*f);
			vec3 at = cr.e.at(p_e, l);
			vec3 n = fa.normal(p_f);
			// TODO: Consolidate plane/vector distance functions
			real d = dot(at - p_f.vertices[fa.verts[0]].v, n);

			if (d > 0) {
				return dot(u, n);
			}

			return -dot(u, n);
		}

		std::optional<feature> deriv_check(
			const polyhedron &p_e,
			const polyhedron &p_f,
			const clip_result &cr
		) {
			if (cr.n1 && edge_dist_deriv(p_e, p_f, cr, cr.l1) > 0) {
				return cr.n1;
			} else if (cr.n2 && edge_dist_deriv(p_e, p_f, cr, cr.l2) < 0) {
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
				real d = dot(v.v - p_f.vertices[fp.verts[0]].v, n);

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
				for (size_t v_i : f.verts) {
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
				if (f.verts.size() < 3) {
					throw geometry_error(f,
						"Face has less than three vertices: " + traits::to_string(f.verts.size())
					);
				}
			}
			// TODO: Verify that the polyhedron is closed and convex
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

		face::face(const std::vector<size_t> &_verts) : verts(_verts) {}

		std::vector<vplane> face::fe_planes(const polyhedron &p) const {
			std::vector<vplane> out{};
			vec3 norm = normal(p);

			for (const edge &e : edges()) {
				vec3 d = normalize(e.h(p).v - e.t(p).v);

				out.push_back(vplane(
					*this,
					e,
					e.t(p).v,
					cross(d, norm)
				));
			}

			return out;
		}

		bool face::has_edge(const edge &e) const {
			auto all_edges = edges();

			return std::find(std::begin(all_edges), std::end(all_edges), e) != std::end(all_edges);
		}

		bool face::has_vertex(size_t i) const {
			return std::find(std::begin(verts), std::end(verts), i) != std::end(verts);
		}

		vec3 face::normal(const polyhedron &p) const {
			vec3 v1 = p.vertices[verts[0]].v;
			vec3 v2 = p.vertices[verts[1]].v;
			vec3 v3 = p.vertices[verts[2]].v;
			vec3 e1 = v2 - v1;
			vec3 e2 = v3 - v2;

			return normalize(cross(e1, e2));
		}

		edge face::get_ccw(const edge &_e) const {
			using traits::to_string;

			for (const edge e : edges()) {
				if (e == _e) {
					return e;
				}
			}

			throw geometry_error(*this, "Edge not found: " + to_string(_e));
		}

		int polyhedron::euler_characteristic() const {
			return (int)(vertices.size() + faces.size()) - (int)edges.size();
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

		// TODO: Consolidate plane/vector distance functions
		real vplane::dist_from(const vec3 &v) const {
			return dot(v - pos, dir);
		}

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
				real vp_violation = dot(v.v - vp.pos, vp.dir);

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
			real dpv = dot(v.v - p_f.vertices[f.verts[0]].v, f_norm);

			for (const edge &e : v.edges(p_v)) {
				vec3 vp = e.v_is[0] == v.i ? e.h(p_v).v : e.t(p_v).v;
				real dpvp = dot(vp - p_f.vertices[f.verts[0]].v, f_norm);

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
					vps = std::get<vertex>(*min_f).ve_planes(p_f);
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
			real d1 = dot(e_v1 - p_f.vertices[f.verts[0]].v, n);
			real d2 = dot(e_v2 - p_f.vertices[f.verts[0]].v, n);

			if (d1 * d2 <= 0) {
				return algorithm_state{
					.f1 = e,
					.f2 = f,
					.step = algorithm_step::Penetration,
					.penetration = d1 < 0 ? d1 : d2
				};
			}

			if (edge_dist_deriv(p_e, p_f, cr, cr.l1) <= 0) {
				if (cr.n1) {
					return algorithm_state{
						.f1 = e,
						.f2 = *cr.n1,
						.step = algorithm_step::Continue,
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
			if (f1.verts.size() != f2.verts.size()) {
				return false;
			}

			size_t i1 = 0;
			size_t i2 = 0;

			for (; i2 < f2.verts.size(); i2++) {
				if (f1.verts[i1] == f2.verts[i2]) {
					break;
				}
			}

			if (i2 == f2.verts.size()) {
				return false;
			}

			for (size_t i = 0; i < f2.verts.size(); i++) {
				size_t j = i + i2;

				if (j >= f2.verts.size()) {
					j -= f2.verts.size();
				}

				if (f1.verts[i] != f2.verts[j]) {
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

	for (size_t i = 0; i < f.verts.size(); i++) {
		out << traits::to_string(f.verts[i]);

		if (i + 1 != f.verts.size()) {
			out << ", ";
		}
	}

	out << ")";

	return out.str();
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