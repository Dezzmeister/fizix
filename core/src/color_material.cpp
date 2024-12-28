#include "color_material.h"
#include "shader_constants.h"
#include "shader_store.h"
#include "util.h"

const std::string color_material::color_shader_name("basic_color");

color_material::color_material(const glm::vec3 &_color) :
	color(_color)
{}

void color_material::prepare_draw(draw_event&, const shader_program &sp, render_pass_state&) const {
	static constexpr int color_loc = util::find_in_map(constants::shader_locs, "color");

	sp.set_uniform(color_loc, color);
}

bool color_material::supports_transparency() const {
	return false;
}

const std::string& color_material::shader_name() const {
	return color_material::color_shader_name;
}
