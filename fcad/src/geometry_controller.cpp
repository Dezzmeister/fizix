#include <random>
#include <color_material.h>
#include <directional_light.h>
#include <phong_color_material.h>
#include <physics/math_util.h>
#include "geometry_controller.h"

using namespace phys::literals;

namespace {
	// Complementary to Windows BSOD blue
	// https://www.canva.com/colors/color-wheel/
	color_material vert_mtl(0xf5d608);
	color_material edge_mtl(0xf5d608);
	phong_color_material face_mtl(
		phong_color_material_properties(
			glm::vec3(0.25, 0.20725, 0.20725),
			glm::vec3(1, 0.829, 0.829),
			glm::vec3(0.296648, 0.296648, 0.296648),
			128 * 0.088f
		)
	);

	std::random_device r{};
	std::default_random_engine rand_engine(r());
	std::uniform_real_distribution<float> float_dist(0.0f, 1.0f);

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
}

geometry_controller::geometry_controller(
	event_buses &_buses,
	fcad_event_bus &_events
) :
	event_listener<new_vertex_event>(&_events),
	event_listener<new_edge_event>(&_events),
	event_listener<new_face_event>(&_events),
	mesh_world(std::make_unique<world>(_buses)),
	vert_geom(std::make_unique<geometry>(
		std::vector<float>({}),
		geometry_primitive_type::Points,
		vbo_usage_hint::DynamicDraw
	)),
	vert_mesh(std::make_unique<mesh>(
		vert_geom.get(),
		&vert_mtl
	)),
	edge_geom(std::make_unique<geometry>(
		std::vector<float>({}),
		geometry_primitive_type::Lines,
		vbo_usage_hint::DynamicDraw
	)),
	edge_mesh(std::make_unique<mesh>(
		edge_geom.get(),
		&edge_mtl
	)),
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
	))
{
	event_listener<new_vertex_event>::subscribe();
	event_listener<new_edge_event>::subscribe();
	event_listener<new_face_event>::subscribe();
	mesh_world->add_mesh(vert_mesh.get());
	mesh_world->add_mesh(edge_mesh.get());
	mesh_world->add_light(sun.get());
	mesh_world->add_light(moon.get());
	sun->set_casts_shadow(false);
	moon->set_casts_shadow(false);
	glPointSize(3);
}

int geometry_controller::handle(new_vertex_event &event) {
	vert_geom->add_vertex(
		event.vertex,
		glm::vec3(0.0f),
		glm::vec2(0.0f)
	);
	poly.add_vertex(event.vertex);

	return 0;
}

int geometry_controller::handle(new_edge_event &event) {
	if (! poly.is_possible_edge(event.e)) {
		logger::debug("Rejecting impossible edge: " + traits::to_string(event.e));

		return 0;
	}

	edge_geom->add_vertex(
		event.e.t(poly).v,
		glm::vec3(0.0f),
		glm::vec2(0.0f)
	);
	edge_geom->add_vertex(
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

		return 0;
	}

	poly.add_face_and_new_edges(event.f);

	std::vector<float> face_verts{};
	triangulate(poly, event.f, face_verts);

	face_geoms.emplace_back(
		std::make_unique<geometry>(
			face_verts,
			geometry_primitive_type::Triangles,
			vbo_usage_hint::StaticDraw
		)
	);

	face_meshes.emplace_back(
		std::make_unique<mesh>(
			face_geoms[face_geoms.size() - 1].get(),
			&face_mtl
		)
	);

	mesh_world->add_mesh(face_meshes[face_meshes.size() - 1].get());

	return 0;
}