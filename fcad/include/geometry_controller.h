#pragma once
#include <draw2d.h>
#include <mesh.h>
#include <world.h>
#include "fcad_events.h"

struct renderable_face : traits::pinned<renderable_face> {
	face f;
	geometry geom;
	mesh m;

	renderable_face(
		const face &_f,
		const material * _mat,
		geometry &&_geom
	) :
		f(_f),
		geom(std::move(_geom)),
		m(&geom, _mat)
	{}
};

class geometry_controller :
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
	std::unique_ptr<world> mesh_world;
	std::unique_ptr<geometry> vert_geom;
	std::unique_ptr<mesh> vert_mesh;
	std::unique_ptr<geometry> edge_geom;
	std::unique_ptr<mesh> edge_mesh;
	std::vector<std::unique_ptr<renderable_face>> face_meshes{};
	std::unique_ptr<light> sun{};
	std::unique_ptr<light> moon{};
	polyhedron poly{};
	mat4 vert_world_to_pre_ndc{};
	const font * vert_label_font{};
	bool show_vert_labels{};

	void regenerate_edge_geom();
	void remove_face_geoms(const std::vector<face> &faces);
};