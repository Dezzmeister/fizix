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

		size_t v1_idx = (size_t)(float_dist(rand_engine) * f.num_verts());
		size_t v2_idx = (size_t)(float_dist(rand_engine) * f.num_verts());

		// We may have to adjust the chosen vertices to ensure that they
		// actually split the polygon
		if (v2_idx == v1_idx) {
			v2_idx += 2;

			if (v2_idx >= f.num_verts()) {
				v2_idx -= f.num_verts();
			}
		} else if (v2_idx == v1_idx + 1) {
			v2_idx++;

			if (v2_idx == f.num_verts()) {
				v2_idx = 0;
			}
		} else if (v1_idx == (f.num_verts() - 1) && v2_idx == 0) {
			v2_idx++;
		} else if (v1_idx == v2_idx + 1) {
			v1_idx++;

			if (v1_idx == f.num_verts()) {
				v1_idx = 0;
			}
		} else if (v1_idx == 0 && v2_idx == (f.num_verts() - 1)) {
			v1_idx++;
		}

		assert(v1_idx < f.num_verts());
		assert(v2_idx < f.num_verts());

		if (v2_idx < v1_idx) {
			std::swap(v1_idx, v2_idx);
		}

		std::vector<size_t> f1_verts{};
		std::vector<size_t> f2_verts{};

		for (size_t i = v1_idx; i <= v2_idx; i++) {
			f1_verts.push_back(f.vert(i));
		}

		for (size_t i = v2_idx; i < f.num_verts(); i++) {
			f2_verts.push_back(f.vert(i));
		}

		for (size_t i = 0; i <= v1_idx; i++) {
			f2_verts.push_back(f.vert(i));
		}

		face f1(f1_verts);
		face f2(f2_verts);

		triangulate(p, f1, out);
		triangulate(p, f2, out);
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

			vec3 v1s = p.vertices[f.vert(vi1)].v;
			vec3 v2e = p.vertices[f.vert(i)].v;

			for (const edge &e : util::concat_views(cut.f1.edges(), cut.f2.edges())) {
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

				if (ts->x > 0.0_r && ts->x < 1.0_r && ts->y > 0.0_r && ts->y < 1.0_r) {
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
	event_listener<new_vertex_event>(&_events),
	event_listener<new_edge_event>(&_events),
	event_listener<new_face_event>(&_events),
	event_listener<delete_vertex_event>(&_events),
	event_listener<delete_edge_event>(&_events),
	event_listener<delete_face_event>(&_events),
	event_listener<post_processing_event>(&_buses.render),
	event_listener<camera_move_event>(&_events),
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
	event_listener<new_vertex_event>::subscribe();
	event_listener<new_edge_event>::subscribe();
	event_listener<new_face_event>::subscribe();
	event_listener<delete_vertex_event>::subscribe();
	event_listener<delete_edge_event>::subscribe();
	event_listener<delete_face_event>::subscribe();
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

void geometry_controller::reset() {
	for (const auto &rf : face_meshes) {
		rf->remove_from_world(mesh_world);
	}

	face_meshes.clear();

	edge_geom.clear_vertices();
	vert_geom.clear_vertices();
	poly.clear();
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

int geometry_controller::handle(program_start_event &event) {
	vert_label_font = &event.draw2d->get_font("spleen_6x12");
	axis_label_font = &event.draw2d->get_font("spleen_12x24");

	return 0;
}

int geometry_controller::handle(fcad_start_event &event) {
	platform = &event.platform;

	return 0;
}

int geometry_controller::handle(new_vertex_event &event) {
	for (const vertex &v : poly.vertices) {
		if (event.vertex == v.v) {
			platform->set_cue_text(L"Vertex already exists");

			return 1;
		}
	}

	vert_geom.add_vertex(
		event.vertex,
		glm::vec3(0.0f),
		glm::vec2(0.0f)
	);
	poly.add_vertex(event.vertex);

	aabb bounds = calculate_aabb();
	float max = std::max(bounds.max_diff(), 2.0f);

	axes.set_max_axis(max / 2.0f);

	return 0;
}

int geometry_controller::handle(new_edge_event &event) {
	if (! poly.is_possible_edge(event.e)) {
		logger::debug("Rejecting impossible edge: " + traits::to_string(event.e));
		platform->set_cue_text(L"Edge refers to a nonexistent vertex");

		return 1;
	}

	if (event.e.v_is[0] == event.e.v_is[1]) {
		logger::debug("Rejecting impossible edge: " + traits::to_string(event.e));
		platform->set_cue_text(L"Edge is degenerate");

		return 1;
	}

	for (const edge &e : poly.edges) {
		if (event.e == e) {
			platform->set_cue_text(L"Edge already exists");

			return 1;
		}
	}

	edge_geom.add_vertex(
		event.e.t(poly).v,
		glm::vec3(0.0f),
		glm::vec2(0.0f)
	);
	edge_geom.add_vertex(
		event.e.h(poly).v,
		glm::vec3(0.0f),
		glm::vec2(0.0f)
	);
	poly.add_edge(event.e);

	return 0;
}

int geometry_controller::handle(new_face_event &event) {
	if (! poly.is_possible_face(event.f)) {
		logger::debug("Rejecting impossible face: " + traits::to_string(event.f));
		platform->set_cue_text(L"Face refers to a nonexistent vertex");

		return 1;
	}

	if (event.f.num_verts() < 3) {
		logger::debug("Rejecting impossible face: " + traits::to_string(event.f));
		platform->set_cue_text(L"Face is degenerate");

		return 1;
	}

	for (size_t i = 0; i < event.f.num_verts() - 1; i++) {
		if (event.f.vert(i) == event.f.vert(i + 1)) {
			logger::debug("Rejecting impossible face: " + traits::to_string(event.f));
			platform->set_cue_text(L"Face contains a degenerate edge");

			return 1;
		}
	}

	for (const face &f : poly.faces) {
		if (event.f == f) {
			platform->set_cue_text(L"Face already exists");

			return 1;
		}
	}

	if (! event.f.is_coplanar(poly)) {
		platform->set_cue_text(L"Face is not coplanar");

		return 1;
	}

	poly.add_face_and_new_edges(event.f);

	std::vector<float> face_verts{};
	try {
		triangulate(poly, event.f, face_verts);
	} catch (geometry_error &err) {
		logger::error("Failed to triangulate " + traits::to_string(event.f) + std::string(err.what()));
		platform->set_cue_text(L"Face is invalid");

		poly.remove_face_and_dead_edges(event.f);

		return 1;
	}

	face_meshes.emplace_back(
		std::make_unique<renderable_face>(
			event.f,
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

	return 0;
}

int geometry_controller::handle(delete_vertex_event &event) {
	if (event.vertex_idx >= poly.vertices.size()) {
		logger::debug("Tried to delete impossible vertex: " + traits::to_string(event.vertex_idx));
		platform->set_cue_text(L"Vertex does not exist");

		return 1;
	}

	std::vector<face> deleted_faces = poly.remove_vertex(event.vertex_idx);

	remove_face_geoms(deleted_faces);

	vert_geom.remove_vertex(event.vertex_idx);
	regenerate_edge_geom();

	for (auto &fm : face_meshes) {
		for (size_t i = 0; i < fm->f.num_verts(); i++) {
			size_t v = fm->f.vert(i);

			if (v > event.vertex_idx) {
				fm->f.set_vert(i, v - 1);
			}
		}
	}

	aabb bounds = calculate_aabb();
	float max = std::max(bounds.max_diff(), 2.0f);

	if (max != infinity) {
		axes.set_max_axis(max / 2.0f);
	}

	return 0;
}

int geometry_controller::handle(delete_edge_event &event) {
	if (! poly.is_possible_edge(event.e)) {
		logger::debug("Tried to delete impossible edge: " + traits::to_string(event.e));
		platform->set_cue_text(L"Edge refers to nonexistent vertex");

		return 1;
	}

	std::vector<face> deleted_faces = poly.remove_edge(event.e);

	remove_face_geoms(deleted_faces);
	regenerate_edge_geom();

	return 0;
}

int geometry_controller::handle(delete_face_event &event) {
	if (! poly.is_possible_face(event.f)) {
		logger::debug("Tried to delete impossible face: " + traits::to_string(event.f));
		platform->set_cue_text(L"Face refers to nonexistent vertex");

		return 1;
	}

	std::vector<face> deleted_faces = poly.remove_face(event.f);
	std::vector<face> deleted_faces_rev = poly.remove_face(event.f.flipped());

	remove_face_geoms(deleted_faces);
	remove_face_geoms(deleted_faces_rev);
	regenerate_edge_geom();

	return 0;
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