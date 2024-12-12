#include "texture_store.h"
#include "constants.h"
#include "particle_utils.h"
#include "connector_spawn_tool.h"

namespace {
	struct tool_props {
		const std::string icon_name;
		const std::string icon_path;
		const std::string tool_name;
		const std::string tooltip;
	};

	const tool_props rod_props = {
		.icon_name = "rod_spawn_tool_icon",
		.icon_path = "assets/icons/new-rod.png",
		.tool_name = "New Rod Tool",
		.tooltip = "Left click two particles to connect them with a rigid beam. Right click to cancel your current selection."
	};

	const tool_props cable_props = {
		.icon_name = "cable_spawn_tool_icon",
		.icon_path = "assets/icons/new-cable.png",
		.tool_name = "New Cable Tool",
		.tooltip = "Left click two particles to connect them with a flexible cable. Right click to cancel your current selection."
	};

	const tool_props& get_tool_props(connector_type type) {
		switch (type) {
			case connector_type::Rod:
				return rod_props;
			case connector_type::Cable:
				return cable_props;
		}

		__assume(false);
	}
}

connector_spawn_tool::connector_spawn_tool(
	event_buses &_buses,
	custom_event_bus &_custom_bus,
	texture_store &_textures,
	world &_mesh_world,
	connector_type _type
) :
	tool(
		_textures.store(get_tool_props(_type).icon_name, texture(get_tool_props(_type).icon_path.c_str())),
		get_tool_props(_type).tool_name,
		get_tool_props(_type).tooltip
	),
	event_listener<pre_render_pass_event>(&_buses.render),
	event_listener<mousedown_event>(&_buses.input),
	event_listener<particle_select_event>(&_custom_bus),
	event_listener<particle_deselect_event>(&_custom_bus),
	buses(_buses),
	custom_bus(_custom_bus),
	mesh_world(_mesh_world),
	type(_type),
	selected_a_mesh(std::make_unique<mesh>(sphere_geom.get(), &selected_sphere_mtl))
{
	event_listener<particle_select_event>::subscribe();
	event_listener<particle_deselect_event>::subscribe();
}

void connector_spawn_tool::activate() {
	event_listener<pre_render_pass_event>::subscribe();
	event_listener<mousedown_event>::subscribe();
}

void connector_spawn_tool::deactivate() {
	particle_a_index = -1;
	particle_b_index = -1;

	if (particle_a) {
		mesh_world.remove_mesh(selected_a_mesh.get());
	}

	particle_a = nullptr;
	particle_b = nullptr;

	event_listener<pre_render_pass_event>::unsubscribe();
	event_listener<mousedown_event>::unsubscribe();
}

bool connector_spawn_tool::is_active() const {
	return event_listener<pre_render_pass_event>::is_subscribed();
}

int connector_spawn_tool::handle(pre_render_pass_event&) {
	if (particle_a) {
		move_mesh_to_particle(selected_a_mesh.get(), particle_a);
		hide_particle(particle_mesh, particle_a_index);
	}

	return 0;
}

int connector_spawn_tool::handle(mousedown_event &event) {
	if (event.button == MOUSE_RIGHT) {
		if (particle_a) {
			mesh_world.remove_mesh(selected_a_mesh.get());
		}

		particle_a = nullptr;
		particle_a_index = -1;
		particle_b = nullptr;
		particle_b_index = -1;

		return 0;
	} else if (event.button == MOUSE_LEFT) {
		if (! selected_particle) {
			return 0;
		}

		if (! particle_a) {
			particle_a_index = selected_particle_index;
			particle_a = selected_particle;

			move_mesh_to_particle(selected_a_mesh.get(), particle_a);
			mesh_world.add_mesh(selected_a_mesh.get());
		} else if (particle_b_index == -1) {
			if (particle_a_index == selected_particle_index) {
				return 0;
			}

			particle_b_index = selected_particle_index;
			particle_b = selected_particle;

			switch (type) {
				case connector_type::Rod: {
					rod_spawn_event rod_event(particle_a_index, particle_b_index);
					custom_bus.fire(rod_event);
					break;
				}
				case connector_type::Cable: {
					cable_spawn_event cable_event(particle_a_index, particle_b_index);
					custom_bus.fire(cable_event);
					break;
				}
			}

			particle_a_index = -1;
			particle_b_index = -1;
			particle_a = nullptr;
			particle_b = nullptr;

			mesh_world.remove_mesh(selected_a_mesh.get());
		}
	}

	return 0;
}

int connector_spawn_tool::handle(particle_select_event &event) {
	selected_particle = &event.p;
	selected_particle_index = event.particle_index;
	particle_mesh = &event.particle_mesh;

	return 0;
}

int connector_spawn_tool::handle(particle_deselect_event&) {
	selected_particle = nullptr;
	selected_particle_index = -1;

	return 0;
}
