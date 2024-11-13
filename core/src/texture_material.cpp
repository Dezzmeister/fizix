#include "texture_material.h"
#include "texture_store.h"

const std::string texture_material::texture_shader_name("basic_texture");

texture_material::texture_material(std::string _texture_name) :
	texture_name(_texture_name)
{}

void texture_material::prepare_draw(draw_event &event, const shader_program &shader, render_pass_state &render_pass) const {
	const texture &tex = event.textures.get(texture_name);
	const int tex_unit = render_pass.next_texture_unit();

	glActiveTexture(GL_TEXTURE0 + tex_unit);
	glBindTexture(GL_TEXTURE_2D, tex.get_id());

	shader.set_uniform("tex", tex_unit);
}

bool texture_material::supports_transparency() const {
	return false;
}

const std::string& texture_material::shader_name() const {
	return texture_material::texture_shader_name;
}
