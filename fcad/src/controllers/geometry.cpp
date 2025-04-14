#include <random>
#include <color_material.h>
#include <directional_light.h>
#include <phong_color_material.h>
#include <physics/math_util.h>
#include "controllers/geometry.h"
#include "fcad_platform/platform.h"

using namespace phys::literals;

namespace {
	// Complementary to Windows BSOD blue
	// https://www.canva.com/colors/color-wheel/
	const color_material vert_mtl(vec3(
		0.9607f,
		0.8392f,
		0.0314f
	));
	const color_material edge_mtl(vec3(
		0.9607f,
		0.8392f,
		0.0314f
	));
	const phong_color_material face_mtl(
		phong_color_material_properties(
			glm::vec3(0.25, 0.20725, 0.20725),
			glm::vec3(1, 0.829, 0.829),
			glm::vec3(0.296648, 0.296648, 0.296648),
			128 * 0.088f
		)
	);
	const phong_color_material face_inv_mtl(
		phong_color_material_properties(
			glm::vec3(0.368, 0.0, 0.306),
			glm::vec3(1, 0.829, 1),
			glm::vec3(0.296648, 0.296648, 0.296648),
			128 * 0.088f
		)
	);

	std::random_device r{};
	std::default_random_engine rand_engine(r());
	std::uniform_real_distribution<float> float_dist(0.0f, 1.0f);

	struct vertex_and_depth {
		vec3 ndc{};
		size_t vert_i{};

		friend bool operator<(const vertex_and_depth &a, const vertex_and_depth &b) {
			return a.ndc.z < b.ndc.z;
		}
	};

	// Turns a polygon into triangles by making successive cuts
	// until only triangles remain
	void triangulate(
		const polyhedron &p,
		const face &f,
		std::vector<float> &out
	);

	void triangulate_convex(
		const polyhedron &p,
		const face &f,
		std::vector<float> &out
	) {
		assert(f.num_verts() >= 3);

		if (f.num_verts() == 3) {
			const vec3 norm = f.normal(p);
			auto verts = f.vertices(p) | std::ranges::to<std::vector<vertex>>();

			geometry::write_vertex(
				out,
				to_glm<vec3>(verts[0].v),
				to_glm<vec3>(norm),
				glm::vec2(0.0f)
			);
			geometry::write_vertex(
				out,
				to_glm<vec3>(verts[1].v),
				to_glm<vec3>(norm),
				glm::vec2(0.0f)
			);
			geometry::write_vertex(
				out,
				to_glm<vec3>(verts[2].v),
				to_glm<vec3>(norm),
				glm::vec2(0.0f)
			);

			return;
		}

		face_cut_result cut = f.cut(0, 2);

		triangulate(p, cut.f1, out);
		triangulate(p, cut.f2, out);
	}

	void triangulate_nonconvex(
		const polyhedron &p,
		const face &f,
		std::vector<float> &out
	) {
		int64_t concave_idx = -1;

		for (size_t i = 0; i < f.num_verts(); i++) {
			if (! f.is_vertex_convex(p, i)) {
				concave_idx = i;
				break;
			}
		}

		if (concave_idx == -1) {
			triangulate_convex(p, f, out);
			return;
		}

		// Pick a concave vertex. The cut must include at least one concave vertex,
		// otherwise a cut may not be possible
		// TODO: Proof?
		size_t vi1 = (size_t)concave_idx;
		vec3 n = f.normal(p);

		for (size_t i = 0; i < f.num_verts(); i++) {
			if (! f.can_make_cut(vi1, i)) {
				continue;
			}

			face_cut_result cut = f.cut(vi1, i);

			cut.f1.fix_colinear_edges(p);
			cut.f2.fix_colinear_edges(p);

			// The cut might create a polygon that lies entirely outside of the
			// original polygon. If that happens, the new face's normal will be flipped
			vec3 n1 = cut.f1.normal(p);
			vec3 n2 = cut.f2.normal(p);

			if (dot(n, n1) < 0) {
				continue;
			}

			if (dot(n, n2) < 0) {
				continue;
			}

			size_t pvi1 = f.vert(vi1);
			size_t pi = f.vert(i);
			vec3 v1s = p.vertices[pvi1].v;
			vec3 v2e = p.vertices[pi].v;

			for (const edge &e : f.edges()) {
				if (e.v_is[0] == pvi1 || e.v_is[0] == pi || e.v_is[1] == pvi1 || e.v_is[1] == pi) {
					continue;
				}

				vec3 ev = e.h(p).v - e.t(p).v;
				std::optional<vec2> ts = phys::line_line_intersection(
					e.t(p).v,
					ev,
					v1s,
					v2e - v1s
				);

				if (! ts) {
					continue;
				}

				if (ts->x >= 0.0_r && ts->x <= 1.0_r && ts->y >= 0.0_r && ts->y <= 1.0_r) {
					// The new edge intersects an existing edge
					goto next_cut;
				}
			}

			triangulate(p, cut.f1, out);
			triangulate(p, cut.f2, out);

			return;

			next_cut:;
		}

		// TODO: Tell the user that the face is invalid
		throw geometry_error(
			f,
			"Unable to find cut for face vertex " + traits::to_string(vi1)
		);
	}

	// Turns a polygon into triangles by making successive cuts
	// until only triangles remain
	void triangulate(
		const polyhedron &p,
		const face &f,
		std::vector<float> &out
	) {
		if (f.is_convex(p)) {
			triangulate_convex(p, f, out);
			return;
		}

		triangulate_nonconvex(p, f, out);
	}

	std::string format_vec3(const vec3 &v) {
		std::stringstream ss{};
		ss << std::setprecision(4);

		ss << "(" << v.x << ",";
		ss << v.y << ",";
		ss << v.z << ")";

		return ss.str();
	}

	size_t add_or_get_vertex(polyhedron &p, const vec3 &vertex) {
		for (size_t i = 0; i < p.vertices.size(); i++) {
			if (p.vertices[i].v == vertex) {
				return i;
			}
		}

		return p.add_vertex(vertex);
	}

	face join_faces(const face &f1, const face &f2, const edge &f1e) {
		std::vector<size_t> new_vs{};

		for (size_t i = 0; i < f1.num_verts(); i++) {
			size_t v1i = f1.vert(i);

			new_vs.push_back(v1i);

			if (v1i == f1e.v_is[0]) {
				size_t v2i1 = 0;
				size_t v2i2 = 0;

				for (size_t j = 0; j < f2.num_verts(); j++) {
					if (f2.vert(j) == f1e.v_is[0]) {
						v2i2 = j;
					} else if (f2.vert(j) == f1e.v_is[1]) {
						v2i1 = j;
					}
				}

				assert(v2i2 != v2i1);

				for (size_t j = v2i2 + 1; j < f2.num_verts(); j++) {
					size_t vi = f2.vert(j);

					if (j == v2i1) {
						continue;
					}

					new_vs.push_back(vi);
				}

				for (size_t j = 0; j < std::min(v2i1, v2i2); j++) {
					assert(j != v2i1);
					assert(j != v2i2);

					new_vs.push_back(f2.vert(j));
				}
			}
		}

		return face(new_vs, convexity::Unspecified);
	}
}

renderable_face::renderable_face(
	const face &_f,
	const material * _mat,
	const material * _inv_mat,
	geometry &&_geom
) :
	f(_f),
	geom(std::move(_geom)),
	m(&geom, _mat),
	inv_m(&geom, _inv_mat, 0, -1, mesh_side::Back)
{}

void renderable_face::add_to_world(world &w) {
	w.add_mesh(&m);
	w.add_mesh(&inv_m);
}

void renderable_face::remove_from_world(world &w) {
	w.remove_mesh(&m);
	w.remove_mesh(&inv_m);
}

geometry_controller::geometry_controller(
	event_buses &_buses,
	fcad_event_bus &_events,
	world &_mesh_world
) :
	event_listener<program_start_event>(&_buses.lifecycle),
	event_listener<fcad_start_event>(&_events),
	event_listener<post_processing_event>(&_buses.render),
	event_listener<camera_move_event>(&_events),
	events(_events),
	mesh_world(_mesh_world),
	vert_geom(
		std::vector<float>({}),
		geometry_primitive_type::Points,
		vbo_usage_hint::DynamicDraw
	),
	vert_mesh(
		&vert_geom,
		&vert_mtl
	),
	edge_geom(
		std::vector<float>({}),
		geometry_primitive_type::Lines,
		vbo_usage_hint::DynamicDraw
	),
	edge_mesh(
		&edge_geom,
		&edge_mtl
	),
	sun(std::make_unique<directional_light>(
		glm::normalize(glm::vec3(0.5f, -1.0f, -0.8f)),
		light_properties(
			glm::vec3(0.6f),
			glm::vec3(0.8f),
			glm::vec3(1.0f)
		)
	)),
	moon(std::make_unique<directional_light>(
		glm::normalize(glm::vec3(-0.8f, 1.0f, 0.5f)),
		light_properties(
			glm::vec3(0.3f),
			glm::vec3(0.5f),
			glm::vec3(1.0f)
		)
	)),
	axes(mesh_world)
{
	event_listener<program_start_event>::subscribe();
	event_listener<fcad_start_event>::subscribe();
	event_listener<post_processing_event>::subscribe();
	event_listener<camera_move_event>::subscribe();

	mesh_world.add_mesh(&vert_mesh);
	mesh_world.add_mesh(&edge_mesh);
	mesh_world.add_light(sun.get());
	mesh_world.add_light(moon.get());
	sun->set_casts_shadow(false);
	moon->set_casts_shadow(false);
	glPointSize(3);
	glLineWidth(2);
}

std::optional<vec3> geometry_controller::get_vertex_pos(size_t vertex_idx) const {
	if (vertex_idx >= poly.vertices.size()) {
		return std::nullopt;
	}

	return poly.vertices[vertex_idx].v;
}

void geometry_controller::reset() {
	for (const auto &rf : face_meshes) {
		rf->remove_from_world(mesh_world);
	}

	face_meshes.clear();

	edge_geom.clear_vertices();
	vert_geom.clear_vertices();
	poly.clear();
}

void geometry_controller::add_triangle(const triangle &tri) {
	size_t v1i = add_or_get_vertex(poly, tri.v1);
	size_t v2i = add_or_get_vertex(poly, tri.v2);
	size_t v3i = add_or_get_vertex(poly, tri.v3);
	face f1({ v1i, v2i, v3i }, convexity::Convex);
	vec3 f1_norm = f1.normal(poly);

	if (dot(f1_norm, tri.normal) < 0.0f) {
		f1 = f1.flipped();
		f1_norm = -f1_norm;
	}

	// Try to join f1 to an existing face. We look for a face with a shared edge
	// where the faces have the same normal, and the edge is in the opposite
	// direction. This is a simple way to join faces, and it doesn't account for
	// faces that share part of an edge.
	// TODO: Partial shared edge
	for (const edge &e : poly.edges) {
		if (! f1.has_edge(e)) {
			continue;
		}

		for (const face &f2 : e.faces(poly)) {
			vec3 f2_norm = f2.normal(poly);

			if (f1_norm != f2_norm) {
				continue;
			}

			edge f1e = f1.get_ccw(e);
			edge f2e = f2.get_ccw(e);

			assert(f1e == f2e);

			// The shared edge must be in the opposite direction on
			// the other face in order for the faces to be joinable
			if (f1e.v_is[0] == f2e.v_is[0]) {
				continue;
			}

			face new_face = join_faces(f1, f2, f1e);

			for (auto &rf : face_meshes) {
				if (rf->f == f2) {
					assert(f1.num_verts() == 3);

					for (size_t vi : f1.verts()) {
						rf->geom.add_vertex(
							poly.vertices[vi].v,
							f1_norm,
							vec2(0.0f)
						);
					}

					rf->f = new_face;

					continue;
				}
			}

			poly.remove_face_and_dead_edges(f2);
			poly.add_face_and_new_edges(new_face);

			return;
		}
	}

	// We couldn't find any face to join f1 with, so we can add f1
	// as-is
	poly.add_face_and_new_edges(f1);
	face_meshes.emplace_back(
		std::make_unique<renderable_face>(
			f1,
			&face_mtl,
			&face_inv_mtl,
			geometry(
				{},
				geometry_primitive_type::Triangles,
				vbo_usage_hint::StaticDraw
			)
		)
	);

	for (size_t vi : f1.verts()) {
		face_meshes[face_meshes.size() - 1]->geom.add_vertex(
			poly.vertices[vi].v,
			f1_norm,
			vec2(0.0f)
		);
	}

	face_meshes[face_meshes.size() - 1]->add_to_world(mesh_world);

	aabb bounds = calculate_aabb();
	float max = std::max(bounds.max_diff(), 2.0f);

	axes.set_max_axis(max / 2.0f);
}

void geometry_controller::set_vert_label_type(vert_label_type _label_type) {
	label_type = _label_type;
}

void geometry_controller::set_vert_labels_visible(bool _visible) {
	show_vert_labels = _visible;
}

bool geometry_controller::are_vert_labels_visible() const {
	return show_vert_labels;
}

const polyhedron& geometry_controller::get_poly() const {
	return poly;
}

std::optional<vec3> geometry_controller::centroid(size_t vertex_idx) const {
	if (vertex_idx >= poly.vertices.size()) {
		logger::debug("Tried to query impossible vertex: " + traits::to_string(vertex_idx));
		platform->set_cue_text(L"Vertex does not exist");

		return std::nullopt;
	}

	return poly.vertices[vertex_idx].v;
}

std::optional<vec3> geometry_controller::centroid(const edge &e) const {
	if (! poly.is_possible_edge(e)) {
		logger::debug("Tried to query impossible edge: " + traits::to_string(e));
		platform->set_cue_text(L"Edge refers to nonexistent vertex");

		return std::nullopt;
	}

	return e.centroid(poly);
}

std::optional<vec3> geometry_controller::centroid(const face &f) const {
	// TODO: Clean up validation
	if (! poly.is_possible_face(f)) {
		logger::debug("Tried to query impossible face: " + traits::to_string(f));
		platform->set_cue_text(L"Face refers to nonexistent vertex");

		return std::nullopt;
	}

	if (f.num_verts() < 3) {
		logger::debug("Tried to query face with < 3 vertices: " + traits::to_string(f));
		platform->set_cue_text(L"Face cannot have less than 3 vertices");

		return std::nullopt;
	}

	return f.centroid(poly);
}

std::optional<vec3> geometry_controller::centroid(const vec3_or_index_feature &feat) const {
	if (std::holds_alternative<vec3>(feat)) {
		return std::get<vec3>(feat);
	} else if (std::holds_alternative<size_t>(feat)) {
		return centroid(std::get<size_t>(feat));
	} else if (std::holds_alternative<edge>(feat)) {
		return centroid(std::get<edge>(feat));
	} else {
		assert(std::holds_alternative<face>(feat));

		std::optional<face> f_opt = get_matching_face(
			std::get<face>(feat), true
		);

		if (! f_opt) {
			platform->set_cue_text(L"Face does not exist");

			return std::nullopt;
		}

		return centroid(*f_opt);
	}
}

std::optional<face> geometry_controller::superset_face(const face &f) const {
	// TODO: Set cue text?
	if (! poly.is_possible_face(f)) {
		return std::nullopt;
	}

	if (f.num_verts() < 3) {
		return std::nullopt;
	}

	return poly.superset_face(f);
}

void geometry_controller::flip(const face &f) {
	if (! poly.is_possible_face(f)) {
		logger::debug("Tried to flip impossible face: " + traits::to_string(f));
		platform->set_cue_text(L"Face refers to nonexistent vertex");

		return;
	}

	if (f.num_verts() < 3) {
		logger::debug("Tried to flip face with < 3 vertices: " + traits::to_string(f));
		platform->set_cue_text(L"Face cannot have less than 3 vertices");

		return;
	}

	std::optional<face> face_to_flip_opt = poly.superset_face(f);

	if (! face_to_flip_opt) {
		face_to_flip_opt = poly.superset_face(f.flipped());
	}

	if (! face_to_flip_opt) {
		logger::debug("Tried to flip nonexistent face: " + traits::to_string(f));
		platform->set_cue_text(L"Face does not exist");

		return;
	}

	const face &face_to_flip = *face_to_flip_opt;
	face flipped = face_to_flip.flipped();

	for (face &pf : poly.faces) {
		if (pf == face_to_flip) {
			pf = flipped;
		}
	}

	for (auto &rf : face_meshes) {
		if (rf->f != face_to_flip) {
			continue;
		}

		rf->f = flipped;
		mesh_side side = rf->m.get_side();

		assert(side != rf->inv_m.get_side());

		if (side == mesh_side::Front) {
			rf->m.set_side(mesh_side::Back);
			rf->inv_m.set_side(mesh_side::Front);
		} else {
			assert(side != mesh_side::Both);

			rf->m.set_side(mesh_side::Front);
			rf->inv_m.set_side(mesh_side::Back);
		}

		return;
	}
}

std::experimental::generator<triangle> geometry_controller::faces() const {
	for (const auto &rf : face_meshes) {
		for (size_t i = 0; i < rf->geom.get_num_vertices(); i += 3) {
			vbo_entry * vbo1 = rf->geom.get_vertex(i);
			vbo_entry * vbo2 = rf->geom.get_vertex(i + 1);
			vbo_entry * vbo3 = rf->geom.get_vertex(i + 2);
			vec3 v1(vbo1->vertex[0], vbo1->vertex[1], vbo1->vertex[2]);
			vec3 v2(vbo2->vertex[0], vbo2->vertex[1], vbo2->vertex[2]);
			vec3 v3(vbo3->vertex[0], vbo3->vertex[1], vbo3->vertex[2]);
			vec3 normal1(vbo1->normal[0], vbo1->normal[1], vbo1->normal[2]);
			vec3 normal2(vbo2->normal[0], vbo2->normal[1], vbo2->normal[2]);
			vec3 normal3(vbo3->normal[0], vbo3->normal[1], vbo3->normal[2]);

			assert(normal1 == normal2);
			assert(normal2 == normal3);

			triangle out = {
				.v1 = v1,
				.v2 = v2,
				.v3 = v3,
				.normal = normal1
			};

			co_yield out;
		}
	}
}

bool geometry_controller::can_create_vertex(const vec3 &pos, bool show_feedback) const {
	for (const vertex &v : poly.vertices) {
		if (pos == v.v) {
			if (show_feedback) {
				platform->set_cue_text(L"Vertex already exists");
			}

			return false;
		}
	}

	return true;
}

bool geometry_controller::can_create_edge(const edge &e, bool show_feedback) const {
	if (! poly.is_possible_edge(e)) {
		if (show_feedback) {
			platform->set_cue_text(L"Edge refers to a nonexistent vertex");
		}

		return false;
	}

	if (e.v_is[0] == e.v_is[1]) {
		if (show_feedback) {
			platform->set_cue_text(L"Edge is degenerate");
		}

		return false;
	}

	for (const edge &pe : poly.edges) {
		if (pe == e) {
			if (show_feedback) {
				platform->set_cue_text(L"Edge already exists");
			}

			return false;
		}
	}

	return true;
}

bool geometry_controller::can_create_face(const face &f, bool show_feedback) const {
	if (f.num_verts() < 3) {
		if (show_feedback) {
			platform->set_cue_text(L"Face is degenerate");
		}

		return false;
	}

	if (! poly.is_possible_face(f)) {
		if (show_feedback) {
			platform->set_cue_text(L"Face refers to a nonexistent vertex");
		}

		return false;
	}

	for (size_t i = 0; i < f.num_verts() - 1; i++) {
		if (f.vert(i) == f.vert(i + 1)) {
			if (show_feedback) {
				platform->set_cue_text(L"Face contains a degenerate edge");
			}

			return false;
		}
	}

	for (const face &pf : poly.faces) {
		if (pf == f) {
			if (show_feedback) {
				platform->set_cue_text(L"Face already exists");
			}

			return false;
		}
	}

	if (! f.is_coplanar(poly)) {
		if (show_feedback) {
			platform->set_cue_text(L"Face is not coplanar");
		}

		return false;
	}

	return true;
}

bool geometry_controller::is_valid_vertex(size_t vertex_idx, bool show_feedback) const {
	if (vertex_idx >= poly.vertices.size()) {
		if (show_feedback) {
			platform->set_cue_text(L"Vertex does not exist");
		}

		return false;
	}

	return true;
}

bool geometry_controller::is_valid_edge(const edge &e, bool show_feedback) const {
	if (! poly.is_possible_edge(e)) {
		if (show_feedback) {
			platform->set_cue_text(L"Edge refers to nonexistent vertex");
		}

		return false;
	}

	if (! poly.has_edge(e)) {
		if (show_feedback) {
			platform->set_cue_text(L"Edge does not exist");
		}

		return false;
	}

	return true;
}

bool geometry_controller::is_well_formed_face(const face &f, bool show_feedback) const {
	if (f.num_verts() < 3) {
		if (show_feedback) {
			platform->set_cue_text(L"Face cannot have less than 3 vertices");
		}

		return false;
	}

	if (! poly.is_possible_face(f)) {
		if (show_feedback) {
			platform->set_cue_text(L"Face refers to nonexistent vertex");
		}

		return false;
	}

	return true;
}

std::optional<face> geometry_controller::get_matching_face(const face &f, bool show_feedback) const {
	if (! is_well_formed_face(f, show_feedback)) {
		return std::nullopt;
	}

	face flipped_f = f.flipped();
	std::optional<face> match_opt = poly.superset_face(f);

	if (! match_opt) {
		match_opt = poly.superset_face(flipped_f);
	}

	if (! match_opt && show_feedback) {
		platform->set_cue_text(L"Face does not exist");
	}

	return match_opt;
}

std::optional<feature> geometry_controller::get_feature(const index_feature &idx, bool show_feedback) const {
	if (std::holds_alternative<size_t>(idx)) {
		size_t vertex_idx = std::get<size_t>(idx);

		if (! is_valid_vertex(vertex_idx, show_feedback)) {
			return std::nullopt;
		}

		return poly.vertices[vertex_idx];
	} else if (std::holds_alternative<edge>(idx)) {
		const edge &e = std::get<edge>(idx);

		if (! is_valid_edge(e, show_feedback)) {
			return std::nullopt;
		}

		return e;
	} else {
		assert(std::holds_alternative<face>(idx));

		return get_matching_face(std::get<face>(idx), show_feedback);
	}
}

void geometry_controller::move_features(const polyhedron &p, const vec3 &offset) {
	for (auto &rf : face_meshes) {
		for (size_t j = 0; j < rf->f.num_verts(); j++) {
			if (! p.has_vertex(poly.vertices[rf->f.vert(j)].v)) {
				continue;
			}

			for (size_t k = 0; k < rf->geom.get_num_vertices(); k++) {
				const vbo_entry * entry = rf->geom.get_vertex(k);
				vec3 fv(entry->vertex[0], entry->vertex[1], entry->vertex[2]);
				vec3 norm(entry->normal[0], entry->normal[1], entry->normal[2]);

				rf->geom.set_vertex(k, fv + offset, norm, vec2(0.0_r));
			}
			break;
		}
	}

	vert_geom.clear_vertices();

	for (size_t i = 0; i < poly.vertices.size(); i++) {
		for (const vertex &v : p.vertices) {
			if (util::eq_within_epsilon(poly.vertices[i].v, v.v)) {
				poly.vertices[i].v += offset;
				break;
			}
		}

		vert_geom.add_vertex(poly.vertices[i].v, vec3(0.0_r), vec2(0.0_r));
	}

	for (int i = (int)poly.vertices.size() - 1; i >= 0; i--) {
		for (int j = i - 1; j >= 0; j--) {
			if (i == j) {
				continue;
			}

			if (util::eq_within_epsilon(poly.vertices[i].v, poly.vertices[j].v)) {
				poly.move_vertex(i, j);
				poly.remove_vertex(i);
			}
		}
	}

	regenerate_edge_geom();
}

void geometry_controller::delete_features(const polyhedron &p) {
	for (int i = (int)face_meshes.size() - 1; i >= 0; i--) {
		auto &rf = face_meshes[i];

		for (size_t j = 0; j < rf->f.num_verts(); j++) {
			if (! p.has_vertex(poly.vertices[rf->f.vert(j)].v)) {
				continue;
			}

			rf->remove_from_world(mesh_world);
			face_meshes.erase(std::begin(face_meshes) + i);
			break;
		}
	}

	for (int i = (int)poly.vertices.size() - 1; i >= 0; i--) {
		for (const vertex &v : p.vertices) {
			if (util::eq_within_epsilon(poly.vertices[i].v, v.v)) {
				vert_geom.remove_vertex(i);
			}
		}
	}

	poly.remove_features(p);
	regenerate_edge_geom();
}

int geometry_controller::handle(program_start_event &event) {
	vert_label_font = &event.draw2d->get_font("spleen_6x12");
	axis_label_font = &event.draw2d->get_font("spleen_12x24");

	return 0;
}

int geometry_controller::handle(fcad_start_event &event) {
	platform = &event.platform;

	return 0;
}

bool geometry_controller::create_vertex(const vec3 &pos, bool send_event) {
	if (! can_create_vertex(pos)) {
		return false;
	}

	if (send_event) {
		create_vertex_event event(pos);

		if (events.fire(event)) {
			return false;
		}
	}

	vert_geom.add_vertex(
		pos,
		glm::vec3(0.0f),
		glm::vec2(0.0f)
	);
	poly.add_vertex(pos);

	aabb bounds = calculate_aabb();
	float max = std::max(bounds.max_diff(), 2.0f);

	axes.set_max_axis(max / 2.0f);

	return true;
}

void geometry_controller::add_poly_at(const polyhedron &p, const vec3 &at) {
	size_t old_num_vertices = poly.vertices.size();
	size_t old_num_edges = poly.edges.size();
	size_t old_num_faces = poly.faces.size();

	polyhedron new_poly = add(poly, p.translated(at));

	for (size_t i = old_num_vertices; i < new_poly.vertices.size(); i++) {
		create_vertex(new_poly.vertices[i].v, false);
	}

	for (size_t i = old_num_edges; i < new_poly.edges.size(); i++) {
		create_edge(new_poly.edges[i], false);
	}

	for (size_t i = old_num_faces; i < new_poly.faces.size(); i++) {
		create_face(new_poly.faces[i], false);
	}

	aabb bounds = calculate_aabb();
	float max = std::max(bounds.max_diff(), 2.0f);

	axes.set_max_axis(max / 2.0f);
}

bool geometry_controller::create_edge(const edge &e, bool send_event) {
	if (! can_create_edge(e)) {
		return false;
	}

	if (send_event) {
		create_edge_event event(e);

		if (events.fire(event)) {
			return false;
		}
	}

	edge_geom.add_vertex(
		e.t(poly).v,
		glm::vec3(0.0f),
		glm::vec2(0.0f)
	);
	edge_geom.add_vertex(
		e.h(poly).v,
		glm::vec3(0.0f),
		glm::vec2(0.0f)
	);
	poly.add_edge(e);

	return true;
}

bool geometry_controller::create_face(const face &f, bool send_event) {
	if (! can_create_face(f)) {
		return false;
	}

	poly.add_face_and_new_edges(f);

	std::vector<float> face_verts{};
	try {
		triangulate(poly, f, face_verts);
	} catch (geometry_error &err) {
		logger::error("Failed to triangulate " + traits::to_string(f) + std::string(err.what()));
		platform->set_cue_text(L"Face is invalid");

		poly.remove_face_and_dead_edges(f);

		return false;
	}

	if (send_event) {
		create_face_event event(f);
		if (events.fire(event)) {
			poly.remove_face_and_dead_edges(f);

			return false;
		}
	}

	face_meshes.emplace_back(
		std::make_unique<renderable_face>(
			f,
			&face_mtl,
			&face_inv_mtl,
			geometry(
				face_verts,
				geometry_primitive_type::Triangles,
				vbo_usage_hint::StaticDraw
			)
		)
	);

	face_meshes[face_meshes.size() - 1]->add_to_world(mesh_world);
	// TODO: Be smarter about this
	regenerate_edge_geom();

	return true;
}

bool geometry_controller::delete_vertex(size_t vertex_idx) {
	if (! is_valid_vertex(vertex_idx)) {
		return false;
	}

	delete_vertex_event event(vertex_idx);
	if (events.fire(event)) {
		return false;
	}

	std::vector<face> deleted_faces = poly.remove_vertex(vertex_idx);

	remove_face_geoms(deleted_faces);

	vert_geom.remove_vertex(vertex_idx);
	regenerate_edge_geom();

	for (auto &fm : face_meshes) {
		for (size_t i = 0; i < fm->f.num_verts(); i++) {
			size_t v = fm->f.vert(i);

			if (v > vertex_idx) {
				fm->f.set_vert(i, v - 1);
			}
		}
	}

	aabb bounds = calculate_aabb();
	float max = std::max(bounds.max_diff(), 2.0f);

	if (max != infinity) {
		axes.set_max_axis(max / 2.0f);
	}

	return true;
}

bool geometry_controller::delete_edge(const edge &e) {
	if (! is_valid_edge(e)) {
		return false;
	}

	delete_edge_event event(e);
	if (events.fire(event)) {
		return false;
	}

	std::vector<face> deleted_faces = poly.remove_edge(e);

	remove_face_geoms(deleted_faces);
	regenerate_edge_geom();

	return true;
}

bool geometry_controller::delete_face(const face &f) {
	std::optional<face> match_opt = get_matching_face(f);

	if (! match_opt) {
		return false;
	}

	const face &face_to_delete = *match_opt;

	delete_face_event event(face_to_delete);
	if (events.fire(event)) {
		return false;
	}

	std::vector<face> deleted_faces = poly.remove_face(face_to_delete);

	remove_face_geoms(deleted_faces);
	regenerate_edge_geom();

	return true;
}

int geometry_controller::handle(post_processing_event &event) {
	assert(vert_label_font);
	axes.draw_labels(event.draw2d, *axis_label_font);

	if (! show_vert_labels) {
		return 0;
	}

	std::vector<vertex_and_depth> verts_and_depths{};

	// TODO: Possibly optimize for performance. We can get OpenGL to do all of this
	// with a little extra work
	for (size_t i = 0; i < poly.vertices.size(); i++) {
		vec4 pre_ndc = vert_world_to_pre_ndc * vec4(poly.vertices[i].v, 1.0f);
		vec3 ndc = vec3(pre_ndc / pre_ndc.w);

		if (
			ndc.x < -1.0f || ndc.x > 1.0f ||
			ndc.y < -1.0f || ndc.y > 1.0f ||
			ndc.z < -1.0f || ndc.z > 1.0f
		) {
			continue;
		}

		vertex_and_depth vd{
			.ndc = ndc,
			.vert_i = i
		};

		verts_and_depths.push_back(vd);
	}

	std::sort(std::begin(verts_and_depths), std::end(verts_and_depths));

	for (const auto &vd : std::ranges::views::reverse(verts_and_depths)) {
		glm::ivec2 screen = event.draw2d.ndc_to_screen(vd.ndc);
		std::string index_str = traits::to_string(vd.vert_i);
		std::string str{};

		switch (label_type) {
			case vert_label_type::IndexOnly: {
				str = index_str;
				break;
			}
			case vert_label_type::IndexAndPos: {
				str = index_str + " " + format_vec3(poly.vertices[vd.vert_i].v);
				break;
			}
		}

		int label_width = vert_label_font->glyph_width * (int)str.size();
		int label_height = vert_label_font->glyph_height;

		event.draw2d.draw_text(
			str,
			*vert_label_font,
			screen.x - label_width / 2,
			screen.y + label_height / 2,
			label_width,
			label_height,
			0,
			vec4(0.0f, 0.0f, 0.0f, 1.0f),
			vec4(0.7f, 0.7f, 0.7f, 0.7f),
			false
		);
	}

	return 0;
}

int geometry_controller::handle(camera_move_event &event) {
	mat4 world_to_pre_ndc = event.proj * event.view;

	axes.set_world_to_pre_ndc(world_to_pre_ndc);
	vert_world_to_pre_ndc = world_to_pre_ndc * vert_mesh.get_model();

	return 0;
}

void geometry_controller::regenerate_edge_geom() {
	edge_geom.clear_vertices();

	for (const edge &e : poly.edges) {
		edge_geom.add_vertex(
			e.t(poly).v,
			glm::vec3(0.0f),
			glm::vec3(0.0f)
		);
		edge_geom.add_vertex(
			e.h(poly).v,
			glm::vec3(0.0f),
			glm::vec3(0.0f)
		);
	}
}

void geometry_controller::remove_face_geoms(const std::vector<face> &faces) {
	for (int i = (int)face_meshes.size() - 1; i >= 0; i--) {
		auto it = std::find(std::begin(faces), std::end(faces), face_meshes[i]->f);

		if (it != std::end(faces)) {
			face_meshes[i]->remove_from_world(mesh_world);
			face_meshes.erase(std::begin(face_meshes) + i);
		}
	}
}

geometry_controller::aabb geometry_controller::calculate_aabb() const {
	vec3 min(infinity);
	vec3 max(-infinity);

	for (const vertex &v : poly.vertices) {
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

	return aabb{
		.min = min,
		.max = max
	};
}

float geometry_controller::aabb::max_diff() const {
	return std::max(
		max.x - min.x,
		std::max(
			max.y - min.y,
			max.z - min.z
		)
	);
}