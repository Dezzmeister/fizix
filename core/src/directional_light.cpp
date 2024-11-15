#include "directional_light.h"
#include "gl.h"
#include "logging.h"
#include "shader_constants.h"
#include "util.h"

static const std::string directional_shadow_map_shader_name("shadow_map");

const directional_shadow_caster_properties directional_light::default_shadow_caster_props(
	glm::vec3(0.0f, 0.0f, -1.0f),
	100.0f,
	1000.0f,
	1024
);

static const glm::vec3 y_up(0.0f, 1.0f, 0.0f);
static const glm::vec3 x_up(1.0f, 0.0f, 0.0f);
static const float EPSILON = 1e-4f;

static constexpr float DEPTH_MAP_BORDER_COLOR[] = {
	1.0f, 1.0f, 1.0f, 1.0f
};

static const glm::vec3& get_world_up(const glm::vec3 &dir) {
	const float len = glm::length(dir);

	if ((len - fabs(glm::dot(dir, y_up))) < EPSILON) {
		return x_up;
	}

	return y_up;
}

directional_shadow_caster_properties::directional_shadow_caster_properties(
	const glm::vec3 &_dir,
	float _frustum_size,
	float _frustum_far,
	int _depth_map_resolution
) :
	eye_pos(glm::vec3(0.0f)),
	dir(_dir),
	view(make_view_mat()),
	proj(glm::ortho(
		-_frustum_size,
		_frustum_size,
		-_frustum_size,
		_frustum_size,
		0.1f,
		_frustum_far
	)),
	view_proj(proj * view),
	depth_map_resolution(_depth_map_resolution)
{}

glm::mat4 directional_shadow_caster_properties::make_view_mat() const {
	const glm::vec3 world_up = get_world_up(-dir);
	const glm::vec3 target = eye_pos + dir;

	return glm::lookAt(eye_pos, target, world_up);
}

void directional_shadow_caster_properties::set_dir(const glm::vec3 &_dir) {
	dir = _dir;
	view = make_view_mat();
	view_proj = proj * view;
}

void directional_shadow_caster_properties::set_eye_pos(const glm::vec3 &_eye_pos) {
	eye_pos = _eye_pos;
	view = make_view_mat();
	view_proj = proj * view;
}

const glm::mat4& directional_shadow_caster_properties::get_mat() const {
	return view_proj;
}

directional_light::directional_light(
	const glm::vec3 _dir,
	const light_properties _props,
	directional_shadow_caster_properties _shadow_props
) :
	light::light(light_type::directional),
	shadow_props(_shadow_props),
	dir(_dir),
	props(_props),
	depth_map(0, [](unsigned int _handle) {
	glDeleteTextures(1, &_handle);
		})
{
	shadow_props.set_dir(dir);
}

void directional_light::set_casts_shadow(bool enabled) {
	if (enabled == casts_shadow()) {
		return;
	}

	if (!enabled) {
		depth_map = 0;
		shadow_fbo = 0;
		return;
	}

	glGenFramebuffers(1, &shadow_fbo);
	glGenTextures(1, &depth_map);
	glBindTexture(GL_TEXTURE_2D, depth_map);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_DEPTH_COMPONENT,
		shadow_props.depth_map_resolution,
		shadow_props.depth_map_resolution,
		0,
		GL_DEPTH_COMPONENT,
		GL_FLOAT,
		NULL
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, DEPTH_MAP_BORDER_COLOR);

	glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_map, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		// TODO: Better errors
		logger::error("Error: framebuffer is not complete");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void directional_light::prepare_shadow_render_pass() const {
	glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo);
	glViewport(0, 0, shadow_props.depth_map_resolution, shadow_props.depth_map_resolution);
	glClear(GL_DEPTH_BUFFER_BIT);
}

void directional_light::prepare_draw(int index, const shader_program &shader, render_pass_state &render_pass) const {
	const int i = (index * light_struct_size) + lights_loc;

	shader.set_uniform(i + type_loc, static_cast<int>(type));
	shader.set_uniform(i + dir_loc, dir);
	shader.set_uniform(i + ambient_loc, props.ambient);
	shader.set_uniform(i + diffuse_loc, props.diffuse);
	shader.set_uniform(i + specular_loc, props.specular);

	const int shadow_i = (index * shadow_caster_struct_size) + shadow_casters_loc;

	shader.set_uniform(shadow_i + shadow_caster_enabled_loc, casts_shadow());

	if (casts_shadow()) {
		shader.set_uniform(shadow_i + light_space_loc, shadow_props.get_mat());

		const int depth_map_tex_unit = render_pass.next_texture_unit();

		glActiveTexture(GL_TEXTURE0 + depth_map_tex_unit);
		glBindTexture(GL_TEXTURE_2D, depth_map);

		shader.set_uniform(shadow_i + depth_map_loc, depth_map_tex_unit);
	} else {
		shader.set_uniform(shadow_i + depth_map_loc, render_pass.default_sampler2d_tex_unit);
	}

	shader.set_uniform(shadow_i + cube_depth_map_loc, render_pass.default_cubesampler_tex_unit);
}

void directional_light::prepare_draw_shadow_map(const shader_program &shader) const {
	static constexpr int view_proj_loc = util::find_in_map(constants::shader_locs, "view_proj");

	shader.set_uniform(view_proj_loc, shadow_props.get_mat());
}

void directional_light::set_dir(const glm::vec3 &_dir) {
	dir = _dir;
	shadow_props.set_dir(dir);
}

const glm::vec3& directional_light::get_dir() const {
	return dir;
}

unsigned int directional_light::get_depth_map_id() const {
	return depth_map;
}

const std::string& directional_light::shadow_map_shader_name() const {
	return directional_shadow_map_shader_name;
}

bool directional_light::is_eq(const light &other) const {
	if (type != other.type) {
		return false;
	}

	const directional_light& dl = static_cast<const directional_light&>(other);

	return (dir == dl.dir) && (props == dl.props);
}