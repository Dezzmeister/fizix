#pragma once
#include <draw2d.h>
#include <mesh.h>
#include <world.h>
#include "fcad_events.h"

struct renderable_face : traits::pinned<renderable_face> {
	face f;
	geometry geom;
	mesh m;
	mesh inv_m;

	renderable_face(
		const face &_f,
		const material * _mat,
		const material * _inv_mat,
		geometry &&_geom
	);

	void add_to_world(world &w);
	void remove_from_world(world &w);
};

class axes_controller : traits::pinned<axes_controller> {
public:
	axes_controller(world &_mesh_world);

	void set_max_axis(float max);
	void set_world_to_pre_ndc(const mat4 &_world_to_pre_ndc);
	void draw_labels(const renderer2d &draw2d, const font &f) const;

private:
	geometry x_axis_geom;
	geometry y_axis_geom;
	geometry z_axis_geom;
	mesh x_axis;
	mesh y_axis;
	mesh z_axis;
	mat4 world_to_pre_ndc{};
	float max_axis{ 1.0f };
	float world_axes_scale{ 1.0f };

	vec3 world_to_ndc(const vec3 &world) const;
};

enum class vert_label_type {
	IndexOnly,
	IndexAndPos
};

struct triangle {
	vec3 v1{};
	vec3 v2{};
	vec3 v3{};
	vec3 normal{};
};

class geometry_controller :
	traits::pinned<geometry_controller>,
	public event_listener<program_start_event>,
	public event_listener<fcad_start_event>,
	public event_listener<post_processing_event>,
	public event_listener<camera_move_event>
{
public:
	geometry_controller(
		event_buses &_buses,
		fcad_event_bus &_events,
		world &_mesh_world
	);

	bool create_vertex(const vec3 &pos);
	bool create_edge(const edge &e);
	bool create_face(const face &f);

	bool delete_vertex(size_t vertex_idx);
	bool delete_edge(const edge &e);
	bool delete_face(const face &f);

	void reset();
	void add_triangle(const triangle &tri);
	void regenerate_edge_geom();

	void set_vert_label_type(vert_label_type _label_type);
	void set_vert_labels_visible(bool _visible);
	bool are_vert_labels_visible() const;

	void flip(const face &f);

	std::experimental::generator<triangle> faces() const;

	int handle(program_start_event &event) override;
	int handle(fcad_start_event &event) override;
	int handle(post_processing_event &event) override;
	int handle(camera_move_event &event) override;

private:
	struct aabb {
		vec3 min{};
		vec3 max{};

		float max_diff() const;
	};

	fcad_event_bus &events;
	platform_bridge * platform{};
	world &mesh_world;
	geometry vert_geom;
	mesh vert_mesh;
	geometry edge_geom;
	mesh edge_mesh;
	std::vector<std::unique_ptr<renderable_face>> face_meshes{};
	std::unique_ptr<light> sun{};
	std::unique_ptr<light> moon{};
	polyhedron poly{};
	mat4 vert_world_to_pre_ndc{};
	const font * vert_label_font{};
	const font * axis_label_font{};
	axes_controller axes;
	vert_label_type label_type{ vert_label_type::IndexAndPos };
	bool show_vert_labels{};

	void remove_face_geoms(const std::vector<face> &faces);
	aabb calculate_aabb() const;
};