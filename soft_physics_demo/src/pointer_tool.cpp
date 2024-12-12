#include "draw2d.h"
#include "phong_color_material.h"
#include "constants.h"
#include "particle_utils.h"
#include "pointer_tool.h"

using namespace phys::literals;

namespace {
	void write_vec3(std::stringstream &ss, const std::string &name, const phys::vec3 &v) {
		ss << name << ": (" << v.x << ", " << v.y << ", " << v.z << ")\n";
	};

	void write_real(std::stringstream &ss, const std::string &name, const phys::real r) {
		ss << name << ": " << r << "\n";
	}
}

pointer_tool::mesh_updater::mesh_updater(event_buses &_buses) :
	event_listener<pre_render_pass_event>(&_buses.render)
{}

int pointer_tool::mesh_updater::handle(pre_render_pass_event&) {
	if (selected_particle) {
		move_mesh_to_particle(selected_particle_mesh, selected_particle);
		hide_particle(particle_mesh, particle_index);
	}

	return 0;
}

pointer_tool::pointer_tool(
	event_buses &_buses,
	custom_event_bus &_custom_bus,
	texture_store &_textures,
	world &_mesh_world
) :
	tool(
		_textures.store("pointer_tool_icon", texture("assets/icons/empty-hand.png")),
		"Pointer Tool",
		"Point at a particle to see its physical properties. "
		"Click and hold a particle (with Left Mouse) to move it around. "
		"Right click a particle to freeze or unfreeze it."
	),
	event_listener<program_start_event>(&_buses.lifecycle),
	event_listener<pre_render_pass_event>(&_buses.render, -15),
	event_listener<post_processing_event>(&_buses.render),
	event_listener<particle_select_event>(&_custom_bus),
	event_listener<particle_deselect_event>(&_custom_bus),
	event_listener<mousedown_event>(&_buses.input),
	event_listener<mouseup_event>(&_buses.input),
	event_listener<player_move_event>(&_buses.player),
	event_listener<player_look_event>(&_buses.player),
	event_listener<player_spawn_event>(&_buses.player),
	mesh_world(_mesh_world),
	selected_particle_mesh(std::make_unique<mesh>(sphere_geom.get(), &selected_sphere_mtl)),
	meshes(_buses)
{
	event_listener<program_start_event>::subscribe();
	event_listener<particle_select_event>::subscribe();
	event_listener<particle_deselect_event>::subscribe();
	event_listener<player_move_event>::subscribe();
	event_listener<player_look_event>::subscribe();
	event_listener<player_spawn_event>::subscribe();
}

void pointer_tool::activate() {
	if (selected_particle) {
		mesh_world.add_mesh(selected_particle_mesh.get());
	}

	event_listener<pre_render_pass_event>::subscribe();
	event_listener<post_processing_event>::subscribe();
	event_listener<mousedown_event>::subscribe();
	event_listener<mouseup_event>::subscribe();
	meshes.event_listener<pre_render_pass_event>::subscribe();
}

void pointer_tool::deactivate() {
	if (selected_particle) {
		mesh_world.remove_mesh(selected_particle_mesh.get());
	}

	if (held_particle) {
		held_particle->set_mass(held_particle_mass);
		held_particle = nullptr;
	}

	event_listener<mouseup_event>::unsubscribe();
	event_listener<mousedown_event>::unsubscribe();
	event_listener<post_processing_event>::unsubscribe();
	event_listener<pre_render_pass_event>::unsubscribe();
	meshes.event_listener<pre_render_pass_event>::unsubscribe();
}

bool pointer_tool::is_active() const {
	return event_listener<pre_render_pass_event>::is_subscribed();
}

int pointer_tool::handle(program_start_event &event) {
	debug_font = &event.draw2d->get_font("spleen_6x12");

	return 0;
}

int pointer_tool::handle(pre_render_pass_event&) {
	if (held_particle) {
		held_particle->pos = phys::vec3(player_pos + (held_particle_dist * player_dir));
	}

	meshes.selected_particle = selected_particle;
	meshes.particle_mesh = particle_mesh;
	meshes.selected_particle_mesh = selected_particle_mesh.get();
	meshes.particle_index = particle_index;

	return 0;
}

int pointer_tool::handle(post_processing_event &event) {
	if (! selected_particle) {
		return 0;
	}

	int width = 280;
	int height = 100;
	int sw = 3;

	event.draw2d.draw_rect(
		0,
		event.screen_height - height,
		width,
		height,
		glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
	);

	event.draw2d.draw_rect(
		sw,
		event.screen_height - height + sw,
		width - (2 * sw),
		height - (2 * sw),
		glm::vec4(1.0f, 1.0f, 0.917f, 1.0f)
	);

	std::stringstream ss{};
	ss << std::setprecision(5);

	write_vec3(ss, "force", selected_particle->force);
	write_vec3(ss, "pos", selected_particle->pos);
	write_vec3(ss, "vel", selected_particle->vel);
	write_vec3(ss, "acc", selected_particle->acc);
	write_real(ss, "mass", selected_particle->get_mass());
	write_real(ss, "radius", selected_particle->radius);

	event.draw2d.draw_text(
		ss.str(),
		*debug_font,
		sw + 2,
		event.screen_height - height + sw + debug_font->glyph_height,
		width - (2 * sw) + 2,
		height - (2 * sw),
		0,
		glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
		glm::vec4(0.0f)
	);

	return 0;
}

int pointer_tool::handle(particle_select_event &event) {
	selected_particle = &event.p;
	particle_index = event.particle_index;
	particle_mesh = &event.particle_mesh;

	move_mesh_to_particle(selected_particle_mesh.get(), selected_particle);

	if (is_active()) {
		mesh_world.add_mesh(selected_particle_mesh.get());
	}

	return 0;
}

int pointer_tool::handle(particle_deselect_event&) {
	selected_particle = nullptr;

	if (is_active()) {
		mesh_world.remove_mesh(selected_particle_mesh.get());
	}

	return 0;
}

int pointer_tool::handle(mousedown_event &event) {
	if (event.button == MOUSE_LEFT) {
		if (selected_particle) {
			held_particle = selected_particle;

			phys::vec3 dx = held_particle->pos - player_pos;
			held_particle_dist = std::sqrt(phys::dot(dx, dx));
			held_particle_mass = held_particle->get_mass();
			held_particle->set_mass(phys::infinity);
		}
	} else if (event.button == MOUSE_RIGHT) {
		if (held_particle) {
			if (frozen_particles.find(held_particle) != std::end(frozen_particles)) {
				held_particle->set_mass(frozen_particles.at(held_particle));
				frozen_particles.erase(held_particle);
			} else {
				frozen_particles[held_particle] = held_particle_mass;
				held_particle_mass = phys::infinity;
				held_particle->vel = phys::vec3(0.0_r);
			}
		} else if (selected_particle) {
			if (frozen_particles.find(selected_particle) != std::end(frozen_particles)) {
				selected_particle->set_mass(frozen_particles.at(selected_particle));
				frozen_particles.erase(selected_particle);
			}
		}
	}

	return 0;
}

int pointer_tool::handle(mouseup_event &event) {
	if (event.button == MOUSE_LEFT) {
		if (held_particle) {
			held_particle->set_mass(held_particle_mass);
		}

		held_particle = nullptr;
	}

	return 0;
}

int pointer_tool::handle(player_move_event &event) {
	player_pos = event.pos;

	return 0;
}

int pointer_tool::handle(player_look_event &event) {
	player_dir = event.dir;

	return 0;
}

int pointer_tool::handle(player_spawn_event &event) {
	player_pos = event.pos;
	player_dir = event.dir;

	return 0;
}
