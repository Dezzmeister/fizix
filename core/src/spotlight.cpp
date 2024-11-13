#include "spotlight.h"

// TODO: Shadow maps for spotlights - a single 90deg fov perspective frustum is
// probably fine
static const std::string SHADOW_MAP_SHADER_NAME("shadow_map");

spotlight::spotlight(
	const glm::vec3 _pos,
	const glm::vec3 _dir,
	float _inner_cutoff,
	float _outer_cutoff,
	const light_properties _props,
	const attenuation_factors _att_factors
) :
	light::light(light_type::spot),
	pos(_pos),
	dir(_dir),
	cos_inner_cutoff(cosf(_inner_cutoff)),
	cos_outer_cutoff(cosf(_outer_cutoff)),
	props(_props),
	att_factors(_att_factors)
{}

void spotlight::prepare_draw(int index, const shader_program &shader, render_pass_state &render_pass) const {
	const int i = (index * light_struct_size) + lights_loc;

	shader.set_uniform(i + type_loc, static_cast<int>(type));
	shader.set_uniform(i + pos_loc, pos);
	shader.set_uniform(i + dir_loc, dir);
	shader.set_uniform(i + ambient_loc, props.ambient);
	shader.set_uniform(i + diffuse_loc, props.diffuse);
	shader.set_uniform(i + specular_loc, props.specular);
	shader.set_uniform(i + inner_cutoff_loc, cos_inner_cutoff);
	shader.set_uniform(i + outer_cutoff_loc, cos_outer_cutoff);
	shader.set_uniform(i + att_c_loc, att_factors.constant);
	shader.set_uniform(i + att_l_loc, att_factors.linear);
	shader.set_uniform(i + att_q_loc, att_factors.quadratic);

	const int shadow_i = (index * shadow_caster_struct_size) + shadow_casters_loc;
	shader.set_uniform(shadow_i + depth_map_loc, render_pass.default_sampler2d_tex_unit);
	shader.set_uniform(shadow_i + cube_depth_map_loc, render_pass.default_cubesampler_tex_unit);
}

// TODO: Implement this
void spotlight::prepare_draw_shadow_map(const shader_program&) const {

}

void spotlight::set_casts_shadow(bool) {
	// TODO: Implement this
}

void spotlight::prepare_shadow_render_pass() const {
	// TODO: Implement this
}

const std::string& spotlight::shadow_map_shader_name() const {
	return SHADOW_MAP_SHADER_NAME;
}

bool spotlight::is_eq(const light &other) const {
	if (type != other.type) {
		return false;
	}

	const spotlight &sl = static_cast<const spotlight&>(other);

	return
		(pos == sl.pos) &&
		(dir == sl.dir) &&
		(cos_inner_cutoff == sl.cos_inner_cutoff) &&
		(cos_outer_cutoff == sl.cos_outer_cutoff) &&
		(props == sl.props) &&
		(att_factors == sl.att_factors);
}
