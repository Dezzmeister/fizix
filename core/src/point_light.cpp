#define _USE_MATH_DEFINES
#include <math.h>
#include "point_light.h"
#include "shader_constants.h"

static const std::string point_shadow_map_shader_name("point_shadow_map");

const point_shadow_caster_properties point_light::default_shadow_caster_properties(
	1024,
	0.1f,
	100.0f
);

point_shadow_caster_properties::point_shadow_caster_properties(
	int _depth_map_resolution,
	float _frustum_near,
	float _frustum_far
) :
	proj(glm::perspective((float)M_PI / 2.0f, 1.0f, _frustum_near, _frustum_far)),
	view(),
	view_proj(),
	depth_map_resolution(_depth_map_resolution),
	frustum_far(_frustum_far)
{
	for (size_t i = 0; i < 6; i++) {
		view[i] = glm::identity<glm::mat4>();
		view_proj[i] = proj;
	}
}

void point_shadow_caster_properties::set_pos(const glm::vec3 &pos) {
	view[0] = glm::lookAt(pos, pos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	view[1] = glm::lookAt(pos, pos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	view[2] = glm::lookAt(pos, pos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	view[3] = glm::lookAt(pos, pos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
	view[4] = glm::lookAt(pos, pos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	view[5] = glm::lookAt(pos, pos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));

	for (size_t i = 0; i < 6; i++) {
		view_proj[i] = proj * view[i];
	}
}

point_light::point_light(
	glm::vec3 _pos,
	light_properties _props,
	attenuation_factors _att_factors,
	point_shadow_caster_properties _shadow_props
) :
	light(light_type::point),
	pos(_pos),
	props(_props),
	att_factors(_att_factors),
	shadow_props(_shadow_props),
	depth_cubemap(0, [](unsigned int _handle) {
	glDeleteTextures(1, &_handle);
		})
{
	shadow_props.set_pos(pos);
}

void point_light::prepare_draw(int index, const shader_program &shader, render_pass_state &render_pass) const {
	const int i = (index * light_struct_size) + lights_loc;

	shader.set_uniform(i + type_loc, static_cast<int>(type));
	shader.set_uniform(i + pos_loc, pos);
	shader.set_uniform(i + ambient_loc, props.ambient);
	shader.set_uniform(i + diffuse_loc, props.diffuse);
	shader.set_uniform(i + specular_loc, props.specular);
	shader.set_uniform(i + att_c_loc, att_factors.constant);
	shader.set_uniform(i + att_l_loc, att_factors.linear);
	shader.set_uniform(i + att_q_loc, att_factors.quadratic);

	const int shadow_i = index * shadow_caster_struct_size;

	shader.set_uniform(shadow_casters_loc + shadow_i + shadow_caster_enabled_loc, casts_shadow());

	if (casts_shadow()) {
		const int tex_id = render_pass.next_texture_unit();

		glActiveTexture(GL_TEXTURE0 + tex_id);
		glBindTexture(GL_TEXTURE_CUBE_MAP, depth_cubemap);

		shader.set_uniform(shadow_casters_loc + shadow_i + light_space_loc, glm::identity<glm::mat4>());
		shader.set_uniform(shadow_casters_loc + shadow_i + cube_depth_map_loc, tex_id);
		shader.set_uniform(shadow_casters_loc + shadow_i + far_plane_loc, shadow_props.frustum_far);
	} else {
		shader.set_uniform(shadow_casters_loc + shadow_i + cube_depth_map_loc, render_pass.default_cubesampler_tex_unit);
	}

	shader.set_uniform(shadow_casters_loc + shadow_i + depth_map_loc, render_pass.default_sampler2d_tex_unit);
}

void point_light::prepare_draw_shadow_map(const shader_program &shader) const {
	static constexpr int view_proj_loc = util::find_in_map(constants::shader_locs, "view_proj");
	static constexpr int light_pos_loc = util::find_in_map(constants::shader_locs, "light_pos");
	static constexpr int shadow_map_far_plane_loc = util::find_in_map(constants::shader_locs, "far_plane");

	for (int i = 0; i < 6; i++) {
		shader.set_uniform(view_proj_loc + i, shadow_props.view_proj[i]);
	}

	shader.set_uniform(light_pos_loc, pos);
	shader.set_uniform(shadow_map_far_plane_loc, shadow_props.frustum_far);
}

void point_light::set_casts_shadow(bool enabled) {
	if (enabled == casts_shadow()) {
		return;
	}

	if (! enabled) {
		depth_cubemap = 0;
		shadow_fbo = 0;
		return;
	}

	glGenFramebuffers(1, &shadow_fbo);
	glGenTextures(1, &depth_cubemap);

	glBindTexture(GL_TEXTURE_CUBE_MAP, depth_cubemap);

	for (int i = 0; i < 6; i++) {
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0,
			GL_DEPTH_COMPONENT,
			shadow_props.depth_map_resolution,
			shadow_props.depth_map_resolution,
			0,
			GL_DEPTH_COMPONENT,
			GL_FLOAT,
			NULL
		);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_cubemap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void point_light::prepare_shadow_render_pass() const {
	glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo);
	glViewport(0, 0, shadow_props.depth_map_resolution, shadow_props.depth_map_resolution);
	glClear(GL_DEPTH_BUFFER_BIT);
}

const glm::vec3& point_light::get_pos() const {
	return pos;
}

void point_light::set_pos(const glm::vec3 &_pos) {
	pos = _pos;
	shadow_props.set_pos(pos);
}

const std::string& point_light::shadow_map_shader_name() const {
	return point_shadow_map_shader_name;
}

unsigned int point_light::get_depth_cubemap_id() const {
	return depth_cubemap;
}

bool point_light::is_eq(const light &other) const {
	if (type != other.type) {
		return false;
	}

	const point_light &pl = static_cast<const point_light&>(other);

	return (pos == pl.pos) && (props == pl.props) && (att_factors == pl.att_factors);
}
