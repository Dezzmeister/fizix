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

class geometry_controller :
	traits::pinned<geometry_controller>,
	public event_listener<program_start_event>,
	public event_listener<new_vertex_event>,
	public event_listener<new_edge_event>,
	public event_listener<new_face_event>,
	public event_listener<delete_vertex_event>,
	public event_listener<delete_edge_event>,
	public event_listener<delete_face_event>,
	public event_listener<keydown_event>,
	public event_listener<post_processing_event>,
	public event_listener<camera_move_event>
{
public:
	geometry_controller(event_buses &_buses, fcad_event_bus &_events);

	void reset();
	void set_vert_label_type(vert_label_type _label_type);
	void set_vert_labels_visible(bool _visible);

	int handle(program_start_event &event) override;
	int handle(new_vertex_event &event) override;
	int handle(new_edge_event &event) override;
	int handle(new_face_event &event) override;
	int handle(delete_vertex_event &event) override;
	int handle(delete_edge_event &event) override;
	int handle(delete_face_event &event) override;
	int handle(keydown_event &event) override;
	int handle(post_processing_event &event) override;
	int handle(camera_move_event &event) override;

private:
	struct aabb {
		vec3 min{};
		vec3 max{};

		float max_diff() const;
	};

	world mesh_world;
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

	void regenerate_edge_geom();
	void remove_face_geoms(const std::vector<face> &faces);
	aabb calculate_aabb() const;
};