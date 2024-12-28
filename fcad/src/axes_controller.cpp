#include <color_material.h>
#include "geometry_controller.h"

namespace {
	const vec3 x_color(1.0f, 0.0f, 0.0f);
	const vec3 y_color(0.9882f, 0.5647f, 0.0118f);
	const vec3 z_color(0.2392f, 0.9882f, 0.0118f);
	const color_material x_axis_mtl(x_color);
	const color_material y_axis_mtl(y_color);
	const color_material z_axis_mtl(z_color);
	const float axis_scale = 1.8f;
}

axes_controller::axes_controller(world &_mesh_world) :
	x_axis_geom({
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f
	}, geometry_primitive_type::Lines),
	y_axis_geom({
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f
	}, geometry_primitive_type::Lines),
	z_axis_geom({
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f
	}, geometry_primitive_type::Lines),
	x_axis(&x_axis_geom, &x_axis_mtl),
	y_axis(&y_axis_geom, &y_axis_mtl),
	z_axis(&z_axis_geom, &z_axis_mtl)
{
	_mesh_world.add_mesh(&x_axis);
	_mesh_world.add_mesh(&y_axis);
	_mesh_world.add_mesh(&z_axis);
}

void axes_controller::set_max_axis(float max) {
	max_axis = max;
	world_axes_scale = max_axis * axis_scale;

	glm::mat4 transform = glm::scale(glm::identity<glm::mat4>(), glm::vec3(world_axes_scale));

	x_axis.set_model(transform);
	y_axis.set_model(transform);
	z_axis.set_model(transform);
}

void axes_controller::set_world_to_pre_ndc(const mat4 &_world_to_pre_ndc) {
	world_to_pre_ndc = _world_to_pre_ndc;
}

void axes_controller::draw_labels(const renderer2d &draw2d, const font &f) const {
	vec3 x_ndc = world_to_ndc(vec3(1.1f, 0.0f, 0.0f) * world_axes_scale);
	glm::ivec2 x_screen = draw2d.ndc_to_screen(x_ndc);
	vec3 y_ndc = world_to_ndc(vec3(0.0f, 1.1f, 0.0f) * world_axes_scale);
	glm::ivec2 y_screen = draw2d.ndc_to_screen(y_ndc);
	vec3 z_ndc = world_to_ndc(vec3(0.0f, 0.0f, 1.1f) * world_axes_scale);
	glm::ivec2 z_screen = draw2d.ndc_to_screen(z_ndc);

	draw2d.draw_text(
		"X",
		f,
		x_screen.x - f.glyph_width / 2,
		x_screen.y + f.glyph_height / 2,
		f.glyph_width,
		f.glyph_height,
		0,
		vec4(x_color, 1.0f),
		vec4(0.0f),
		false,
		// This depends on glDepthRange
		(x_ndc.z + 1.0f) / 2.0f
	);

	draw2d.draw_text(
		"Y",
		f,
		y_screen.x - f.glyph_width / 2,
		y_screen.y + f.glyph_height / 2,
		f.glyph_width,
		f.glyph_height,
		0,
		vec4(y_color, 1.0f),
		vec4(0.0f),
		false,
		(y_ndc.z + 1.0f) / 2.0f
	);

	draw2d.draw_text(
		"Z",
		f,
		z_screen.x - f.glyph_width / 2,
		z_screen.y + f.glyph_height / 2,
		f.glyph_width,
		f.glyph_height,
		0,
		vec4(z_color, 1.0f),
		vec4(0.0f),
		false,
		(z_ndc.z + 1.0f) / 2.0f
	);
}

vec3 axes_controller::world_to_ndc(const vec3 &world) const {
	vec4 pre_ndc = world_to_pre_ndc * vec4(world, 1.0f);

	return vec3(pre_ndc / pre_ndc.w);
}