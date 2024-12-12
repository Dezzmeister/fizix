#include <memory>
#include "phong_color_material.h"
#include "shapes.h"
#include "constants.h"
#include "particle_spawn_tool.h"

namespace {
	const float min_scroll_f = 0.2f;
	const float max_scroll_f = 10000.0f;
	const float scroll_f_step = 0.1f;
}

particle_spawn_tool::particle_spawn_tool(
	event_buses &_buses,
	custom_event_bus &_custom_bus,
	texture_store &_textures,
	world &_w
) :
	tool(
		_textures.store("particle_spawn_tool_icon", texture("assets/icons/new-particle.png")),
		"New Particle Tool",
		"Left Click to spawn a particle. Hold Right Mouse and scroll to move the 3D cursor in and out."
	),
	event_listener<player_look_event>(&_buses.player),
	event_listener<player_move_event>(&_buses.player),
	event_listener<player_spawn_event>(&_buses.player),
	event_listener<mousedown_event>(&_buses.input),
	event_listener<mouseup_event>(&_buses.input),
	event_listener<mouse_scroll_event>(&_buses.input),
	event_listener<pre_render_pass_event>(&_buses.render),
	event_listener<tool_select_event>(&_custom_bus, -10),
	custom_bus(_custom_bus),
	sphere_geom(std::make_unique<geometry>(shapes::make_sphere(20, 10, true))),
	preview(std::make_unique<mesh>(sphere_geom.get(), &sphere_mtl)),
	w(_w)
{
	event_listener<player_look_event>::subscribe();
	event_listener<player_move_event>::subscribe();
	event_listener<player_spawn_event>::subscribe();

	preview->set_alpha(0.4f);
}

void particle_spawn_tool::activate() {
	event_listener<mousedown_event>::subscribe();
	event_listener<mouseup_event>::subscribe();
	event_listener<mouse_scroll_event>::subscribe();
	event_listener<pre_render_pass_event>::subscribe();
	event_listener<tool_select_event>::subscribe();

	w.add_mesh(preview.get());
}

void particle_spawn_tool::deactivate() {
	event_listener<mousedown_event>::unsubscribe();
	event_listener<mouseup_event>::unsubscribe();
	event_listener<mouse_scroll_event>::unsubscribe();
	event_listener<pre_render_pass_event>::unsubscribe();
	event_listener<tool_select_event>::unsubscribe();

	w.remove_mesh(preview.get());
}

bool particle_spawn_tool::is_active() const {
	return event_listener<pre_render_pass_event>::is_subscribed();
}

int particle_spawn_tool::handle(player_look_event &event) {
	dir = event.dir;

	return 0;
}

int particle_spawn_tool::handle(player_move_event &event) {
	player_pos = event.pos;

	return 0;
}

int particle_spawn_tool::handle(player_spawn_event &event) {
	player_pos = event.pos;
	dir = event.dir;

	return 0;
}

int particle_spawn_tool::handle(mousedown_event &event) {
	if (! event.is_mouse_locked) {
		return 0;
	}

	if (event.button == MOUSE_LEFT) {
		particle_spawn_event spawn_event(player_pos + (dir * scroll_f));
		custom_bus.fire(spawn_event);
	} else if (event.button == MOUSE_RIGHT) {
		can_zoom = true;
	}

	return 0;
}

int particle_spawn_tool::handle(mouseup_event &event) {
	if (event.button == MOUSE_RIGHT) {
		can_zoom = false;
	}

	return 0;
}

int particle_spawn_tool::handle(mouse_scroll_event &event) {
	if (! can_zoom || ! event.is_mouse_locked) {
		return 0;
	}

	scroll_f += event.v_delta * scroll_f_step / 90.0f;
	scroll_f = std::clamp(scroll_f, min_scroll_f, max_scroll_f);

	return 0;
}

int particle_spawn_tool::handle(pre_render_pass_event&) {
	const glm::mat4 trans_part = glm::translate(
		glm::identity<glm::mat4>(),
		player_pos + (dir * scroll_f)
	);

	preview->set_model(trans_part * sphere_scale);

	return 0;
}

int particle_spawn_tool::handle(tool_select_event&) {
	if (can_zoom) {
		// This cancels the event and prevents the toolbox from switching
		// to the next tool so that we can scroll freely while RMB is held
		return 1;
	}

	return 0;
}