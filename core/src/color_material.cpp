#include "color_material.h"
#include "shader_store.h"

const std::string color_material::color_shader_name("basic_color");

color_material::color_material(unsigned int _color) :
	color(_color),
	color_vec(
		(float)((color & 0x00ff0000) >> 16) / 255.0f,
		(float)((color & 0x0000ff00) >> 8) / 255.0f,
		(float)(color & 0x000000ff) / 255.0f
	)
{}

void color_material::prepare_draw(draw_event&, const shader_program &sp, render_pass_state&) const {
	sp.set_uniform("color", color_vec);
}

bool color_material::supports_transparency() const {
	return false;
}

const std::string& color_material::shader_name() const {
	return color_material::color_shader_name;
}
