#pragma once
#include "light.h"

class directional_light;

class directional_shadow_caster_properties {
private:
	glm::vec3 eye_pos;
	glm::vec3 dir;
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 view_proj;
	int depth_map_resolution;

	void set_dir(const glm::vec3 &_dir);

	glm::mat4 make_view_mat() const;

	friend class directional_light;

public:
	directional_shadow_caster_properties(
		const glm::vec3 &_dir,
		float _frustum_size,
		float _frustum_far,
		int _depth_map_resolution
	);

	void set_eye_pos(const glm::vec3 &_eye_pos);

	const glm::mat4& get_mat() const;
};

class directional_light : public light {
public:
	directional_shadow_caster_properties shadow_props;

	directional_light(
		const glm::vec3 _dir,
		const light_properties _props,
		directional_shadow_caster_properties _shadow_props = default_shadow_caster_props
	);

	void prepare_draw(int index, const shader_program &shader, render_pass_state &render_pass) const override;
	void prepare_draw_shadow_map(const shader_program &shader) const override;
	void prepare_shadow_render_pass() const override;

	void set_dir(const glm::vec3 &_dir);
	void set_casts_shadow(bool enabled) override;

	const glm::vec3& get_dir() const;
	unsigned int get_depth_map_id() const;
	const std::string& shadow_map_shader_name() const override;

protected:

	bool is_eq(const light &other) const override;

private:
	static const directional_shadow_caster_properties default_shadow_caster_props;

	glm::vec3 dir;
	light_properties props;
	unique_handle<unsigned int> depth_map;
};
