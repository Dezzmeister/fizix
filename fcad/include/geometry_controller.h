#pragma once
#include <mesh.h>
#include <world.h>
#include "fcad_events.h"

class geometry_controller :
	public event_listener<new_vertex_event>,
	public event_listener<new_edge_event>,
	public event_listener<new_face_event>
{
public:
	geometry_controller(event_buses &_buses, fcad_event_bus &_events);

	int handle(new_vertex_event &event) override;
	int handle(new_edge_event &event) override;
	int handle(new_face_event &event) override;

private:
	std::unique_ptr<world> mesh_world;
	std::unique_ptr<geometry> vert_geom;
	std::unique_ptr<material> vert_mtl;
	std::unique_ptr<mesh> vert_mesh;
	std::unique_ptr<geometry> edge_geom;
	std::unique_ptr<material> edge_mtl;
	std::unique_ptr<mesh> edge_mesh;
	std::vector<std::unique_ptr<geometry>> face_geoms{};
	std::unique_ptr<material> face_mtl;
	std::vector<std::unique_ptr<mesh>> face_meshes{};
	polyhedron poly{};
};