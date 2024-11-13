#pragma once
#include <glm/glm.hpp>
#include <string>
#include "rendering.h"
#include "shader_constants.h"
#include "shader_program.h"
#include "util.h"

struct light_properties {
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;

	light_properties(glm::vec3 _ambient, glm::vec3 _diffuse, glm::vec3 _specular);

	friend bool operator==(const light_properties &a, const light_properties &b);
};

struct attenuation_factors {
	float constant;
	float linear;
	float quadratic;

	attenuation_factors(float _constant, float _linear, float _quadratic);

	friend bool operator==(const attenuation_factors &a, const attenuation_factors &b);
};

enum class light_type : int {
	point = 0,
	directional = 1,
	spot = 2
};

class world;

class light {
public:
	// We only support 20 lights for now. Uniform sizes have to be known at compile time,
	// so there's no easy way to pass a variable number of lights into a shader. 20 lights
	// should be more than enough for now. If I need more, I'll increase this limit and measure
	// the performance impact.
	// TODO: Uniform buffer objects?
	static constexpr int max_lights = 20;

	const light_type type;

	light(light_type _type);
	virtual ~light() = default;

	virtual void prepare_draw(int index, const shader_program &shader, render_pass_state &render_pass) const = 0;
	virtual void prepare_draw_shadow_map(const shader_program &shader) const = 0;
	virtual void prepare_shadow_render_pass() const = 0;

	virtual void set_casts_shadow(bool enabled) = 0;

	bool casts_shadow() const;
	unsigned int get_shadow_fbo() const;
	virtual const std::string& shadow_map_shader_name() const = 0;

	friend bool operator==(const light &a, const light &b);

	friend class world;

protected:
	// TODO: Remove these and write a method in shader_program that does the lookup
	static constexpr int num_lights_loc = util::find_in_map(constants::shader_locs, "num_lights");
	static constexpr int lights_loc = util::find_in_map(constants::shader_locs, "lights");
	static constexpr int type_loc = util::find_in_map(constants::shader_locs, "light.type");
	static constexpr int pos_loc = util::find_in_map(constants::shader_locs, "light.pos");
	static constexpr int dir_loc = util::find_in_map(constants::shader_locs, "light.dir");
	static constexpr int ambient_loc = util::find_in_map(constants::shader_locs, "light.ambient");
	static constexpr int diffuse_loc = util::find_in_map(constants::shader_locs, "light.diffuse");
	static constexpr int specular_loc = util::find_in_map(constants::shader_locs, "light.specular");
	static constexpr int att_c_loc = util::find_in_map(constants::shader_locs, "light.att_c");
	static constexpr int att_l_loc = util::find_in_map(constants::shader_locs, "light.att_l");
	static constexpr int att_q_loc = util::find_in_map(constants::shader_locs, "light.att_q");
	static constexpr int inner_cutoff_loc = util::find_in_map(constants::shader_locs, "light.inner_cutoff");
	static constexpr int outer_cutoff_loc = util::find_in_map(constants::shader_locs, "light.outer_cutoff");
	static constexpr int light_struct_size = util::find_in_map(constants::shader_locs, "sizeof(light)");
	static constexpr int shadow_casters_loc = util::find_in_map(constants::shader_locs, "shadow_casters");
	static constexpr int light_space_loc = util::find_in_map(constants::shader_locs, "shadow_caster.light_space");
	static constexpr int depth_map_loc = util::find_in_map(constants::shader_locs, "shadow_caster.depth_map");
	static constexpr int shadow_caster_enabled_loc = util::find_in_map(constants::shader_locs, "shadow_caster.enabled");
	static constexpr int cube_depth_map_loc = util::find_in_map(constants::shader_locs, "shadow_caster.cube_depth_map");
	static constexpr int far_plane_loc = util::find_in_map(constants::shader_locs, "shadow_caster.far_plane");
	static constexpr int shadow_caster_struct_size = util::find_in_map(constants::shader_locs, "sizeof(shadow_caster)");

	unique_handle<unsigned int> shadow_fbo;

	virtual bool is_eq(const light &other) const = 0;
};

