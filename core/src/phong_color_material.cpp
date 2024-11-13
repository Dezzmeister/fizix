#include "phong_color_material.h"
#include "point_light.h"
#include "shader_constants.h"
#include "util.h"

const std::string phong_color_material::phong_shader_name("phong_color");

void phong_color_material::prepare_draw(draw_event&, const shader_program &shader, render_pass_state&) const {
	static constexpr int ambient_loc = util::find_in_map(constants::shader_locs, "_color_mat.ambient");
	static constexpr int diffuse_loc = util::find_in_map(constants::shader_locs, "_color_mat.diffuse");
	static constexpr int specular_loc = util::find_in_map(constants::shader_locs, "_color_mat.specular");
	static constexpr int shininess_loc = util::find_in_map(constants::shader_locs, "_color_mat.shininess");

	shader.set_uniform(ambient_loc, mat.ambient);
	shader.set_uniform(diffuse_loc, mat.diffuse);
	shader.set_uniform(specular_loc, mat.specular);
	shader.set_uniform(shininess_loc, mat.shininess);
}

bool phong_color_material::supports_transparency() const {
	return true;
}

const std::string& phong_color_material::shader_name() const {
	return phong_color_material::phong_shader_name;
}
