#include <random>
#include <color_material.h>
#include <phong_color_material.h>
#include "geometry_controller.h"

namespace {
	std::random_device r{};
	std::default_random_engine rand_engine(r());
	std::uniform_real_distribution<float> float_dist(0.0f, 1.0f);

	// Turns a polygon into triangles by making successive cuts
	// until only triangles remain
	void triangulate(
		const polyhedron &p,
		const face &f,
		std::vector<float> &out
	) {
		assert(f.verts.size() >= 3);

		if (f.verts.size() == 3) {
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

		size_t v1_idx = (size_t)(float_dist(rand_engine) * f.verts.size());
		size_t v2_idx = (size_t)(float_dist(rand_engine) * f.verts.size());

		// We may have to adjust the chosen vertices to ensure that they
		// actually split the polygon
		if (v2_idx == v1_idx) {
			v2_idx += 2;

			if (v2_idx >= f.verts.size()) {
				v2_idx -= f.verts.size();
			}
		} else if (v2_idx == v1_idx + 1) {
			v2_idx++;

			if (v2_idx == f.verts.size()) {
				v2_idx = 0;
			}
		} else if (v1_idx == (f.verts.size() - 1) && v2_idx == 0) {
			v2_idx++;
		} else if (v1_idx == v2_idx + 1) {
			v1_idx++;

			if (v1_idx == f.verts.size()) {
				v1_idx = 0;
			}
		} else if (v1_idx == 0 && v2_idx == (f.verts.size() - 1)) {
			v1_idx++;
		}

		assert(v1_idx < f.verts.size());
		assert(v2_idx < f.verts.size());

		if (v2_idx < v1_idx) {
			std::swap(v1_idx, v2_idx);
		}

		std::vector<size_t> f1_verts{};
		std::vector<size_t> f2_verts{};

		for (size_t i = v1_idx; i <= v2_idx; i++) {
			f1_verts.push_back(f.verts[i]);
		}

		for (size_t i = v2_idx; i < f.verts.size(); i++) {
			f2_verts.push_back(f.verts[i]);
		}

		for (size_t i = 0; i <= v1_idx; i++) {
			f2_verts.push_back(f.verts[i]);
		}

		face f1(f1_verts);
		face f2(f2_verts);

		triangulate(p, f1, out);
		triangulate(p, f2, out);
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
	vert_mtl(std::make_unique<color_material>(
		// Complementary to Windows BSOD blue
		// https://www.canva.com/colors/color-wheel/
		0xf5d608
	)),
	vert_mesh(std::make_unique<mesh>(
		vert_geom.get(),
		vert_mtl.get()
	)),
	edge_geom(std::make_unique<geometry>(
		std::vector<float>({}),
		geometry_primitive_type::Lines,
		vbo_usage_hint::DynamicDraw
	)),
	edge_mtl(std::make_unique<color_material>(
		0xf5d608
	)),
	edge_mesh(std::make_unique<mesh>(
		edge_geom.get(),
		edge_mtl.get()
	)),
	face_mtl(std::make_unique<color_material>(
		0xf5d608
	))
{

	event_listener<new_vertex_event>::subscribe();
	event_listener<new_edge_event>::subscribe();
	event_listener<new_face_event>::subscribe();
	mesh_world->add_mesh(vert_mesh.get());
	mesh_world->add_mesh(edge_mesh.get());
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
			face_mtl.get()
		)
	);

	mesh_world->add_mesh(face_meshes[face_meshes.size() - 1].get());

	return 0;
}