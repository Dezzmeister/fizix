#include "shapes.h"
#include "texture_store.h"
#include "pointer_tool.h"
#include "particle_spawn_tool.h"
#include "connector_spawn_tool.h"
#include "tools.h"

tool::tool(
	const texture &_icon,
	const std::string &_name,
	const std::string &_tooltip
) :
	icon(_icon),
	name(_name),
	tooltip(_tooltip)
{}

toolbox::toolbox(
	event_buses &_buses,
	custom_event_bus &_custom_bus,
	world &_mesh_world
) :
	event_listener<program_start_event>(&_buses.lifecycle),
	event_listener<mouse_scroll_event>(&_buses.input),
	buses(_buses),
	custom_bus(_custom_bus),
	mesh_world(_mesh_world)
{
	event_listener<program_start_event>::subscribe();
	event_listener<mouse_scroll_event>::subscribe();
}

int toolbox::handle(program_start_event &event) {
	tools.push_back(std::make_unique<pointer_tool>(
		buses,
		custom_bus,
		*event.textures,
		mesh_world
	));

	tools.push_back(std::make_unique<particle_spawn_tool>(
		buses,
		custom_bus,
		*event.textures,
		mesh_world
	));

	tools.push_back(std::make_unique<connector_spawn_tool>(
		buses,
		custom_bus,
		*event.textures,
		mesh_world,
		connector_type::Rod
	));

	tools.push_back(std::make_unique<connector_spawn_tool>(
		buses,
		custom_bus,
		*event.textures,
		mesh_world,
		connector_type::Cable
	));

	for (size_t i = 0; i < tools.size(); i++) {
		tool_register_event register_event(tools[i].get());
		custom_bus.fire(register_event);
	}

	tool_select_event select_event(tools[curr_tool].get());
	custom_bus.fire(select_event);
	tools[curr_tool]->activate();

	return 0;
}

int toolbox::handle(mouse_scroll_event &event) {
	if (! tools.size() || event.offset.y == 0.0f) {
		return 0;
	}

	size_t old_tool = curr_tool;
	size_t new_tool = curr_tool;

	if (event.offset.y > 0) {
		if (new_tool == 0) {
			new_tool = tools.size() - 1;
		} else {
			new_tool--;
		}
	} else {
		new_tool = (new_tool + 1) % tools.size();
	}

	tool_select_event select_event(tools[new_tool].get());
	int status = custom_bus.fire(select_event);

	if (! status) {
		if (tools[old_tool]->is_active()) {
			tools[old_tool]->deactivate();
		}

		tools[new_tool]->activate();
		curr_tool = new_tool;
	}

	return 0;
}

void toolbox::register_tool(const tool * t) {
	tool_register_event register_event(t);
	custom_bus.fire(register_event);
}