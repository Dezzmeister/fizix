#define _USE_MATH_DEFINES
#include <math.h>
#include <glm/gtc/quaternion.hpp>
#include <color_material.h>
#include <logging.h>
#include <phong_color_material.h>
#include <shapes.h>
#include <util.h>
#include "commands.h"
#include "helpers.h"
#include "parameter/parser.h"
#include "parsing.h"

using namespace phys;

namespace {
	const phong_color_material plane_preview_mtl(
		phong_color_material_properties(
			glm::vec3(1.0f, 0.99f, 0.67f),
			glm::vec3(1.0f, 0.97f, 0.17f),
			glm::vec3(0.296648, 0.296648, 0.296648),
			128 * 0.088f
		)
	);

	const color_material line_preview_mtl(vec3(1.0f, 0.97f, 0.33f));
	const color_material vert_preview_mtl(vec3(1.0f, 0.97f, 0.33f));
}

create_vertex_preview::create_vertex_preview(
	mesh &&_preview,
	const mat4 &_base_transform
) :
	preview(std::move(_preview)),
	base_transform(_base_transform)
{}

void create_vertex_preview::show(world &w, const vec3 &at) {
	preview.set_model(glm::translate(glm::identity<glm::mat4>(), at) * base_transform);

	if (is_visible) {
		return;
	}

	w.add_mesh(&preview);
	is_visible = true;
}

void create_vertex_preview::hide(world &w) {
	if (! is_visible) {
		return;
	}

	w.remove_mesh(&preview);
	is_visible = false;
}

create_vertex_command_impl::create_vertex_command_impl(fcad_event_bus &_events) :
	noop_command_impl(_events),
	vert_geom(std::vector<float>({
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f
	}), geometry_primitive_type::Points, vbo_usage_hint::DynamicDraw),
	line_geom(std::vector<float>({
		0.0f, 0.0f, 100.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 0.0f, -100.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f
	}), geometry_primitive_type::Lines, vbo_usage_hint::DynamicDraw),
	vert_preview(mesh(&vert_geom, &vert_preview_mtl)),
	line_preview(mesh(&line_geom, &line_preview_mtl)),
	plane_preview(mesh(shapes::plane.get(), &plane_preview_mtl, 0, -1, mesh_side::Both),
		glm::mat4_cast(glm::quat(
			std::cos((float)M_PI / 4.0f), (float)std::sin((float)M_PI / 4.0f) * vec3(0.0f, 0.0f, 1.0f)
		)) * glm::scale(glm::identity<glm::mat4>(), glm::vec3(100.0f))
	)
{
	line_preview.preview.set_alpha(0.3f);
	plane_preview.preview.set_alpha(0.3f);
}

void create_vertex_command_impl::on_cancel(const std::wstring&) {
	vert_preview.hide(*mesh_world);
	line_preview.hide(*mesh_world);
	plane_preview.hide(*mesh_world);
}

void create_vertex_command_impl::on_input(const std::wstring &args_buf) {
	std::wstringstream wss{};
	wss << args_buf;
	parsing::parser_state state(wss);
	parsing::parse_whitespace(state);

	partial_vec3 pos_opt = parse_vec3(state);

	if (pos_opt.x && ! pos_opt.y) {
		float x = *pos_opt.x;

		plane_preview.show(*mesh_world, vec3(x, 0.0f, 0.0f));
		vert_preview.hide(*mesh_world);
		line_preview.hide(*mesh_world);

		return;
	} else if (pos_opt.x && pos_opt.y && ! pos_opt.z) {
		float x = *pos_opt.x;
		float y = *pos_opt.y;

		line_preview.show(*mesh_world, vec3(x, y, 0.0f));
		plane_preview.hide(*mesh_world);
		vert_preview.hide(*mesh_world);
	} else if (std::optional<vec3> v = pos_opt.try_as_vec3()) {
		vert_preview.show(*mesh_world, *v);
		line_preview.hide(*mesh_world);
		plane_preview.hide(*mesh_world);
	} else {
		vert_preview.hide(*mesh_world);
		line_preview.hide(*mesh_world);
		plane_preview.hide(*mesh_world);
	}
}

void create_vertex_command_impl::on_submit(const std::wstring &args) {
	vert_preview.hide(*mesh_world);
	line_preview.hide(*mesh_world);
	plane_preview.hide(*mesh_world);

	std::wstringstream wss{};
	wss << args;
	parsing::parser_state state(wss);
	error_log log{};
	parsing::parse_whitespace(state);

	std::optional<std::unique_ptr<vector_expr>> expr_opt = parse_vector_expr(
		state,
		log,
		true
	);

	if (! expr_opt) {
		set_output(log.to_wstr(args));
		return;
	}

	if (params->create_vertex(std::move(*expr_opt))) {
		history->add_command(L":v " + args);
	}
}

void create_vertex_command_impl::write_help_text(std::ostream &os) const {
	write_help_rtf_row(os, ":v <x> <y> <z>",
		"Creates a vertex given X, Y, and Z coordinates."
	);
}

int create_vertex_command_impl::handle(fcad_start_event &event) {
	noop_command_impl::handle(event);
	mesh_world = &event.mesh_world;

	return 0;
}