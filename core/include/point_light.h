#pragma once
#include "light.h"

class point_light;

class point_shadow_caster_properties {
private:
	glm::mat4 proj;
	glm::mat4 view[6];
	glm::mat4 view_proj[6];
	int depth_map_resolution;
	float frustum_far;

	friend class point_light;

public:
	point_shadow_caster_properties(
		int _depth_map_resolution,
		float _frustum_near,
		float _frustum_far
	);

	void set_pos(const glm::vec3 &pos);
};

class point_light : public light {
public:
	glm::vec3 pos;
	light_properties props;
	attenuation_factors att_factors;
	point_shadow_caster_properties shadow_props;

	point_light(
		glm::vec3 _pos,
		light_properties _props,
		attenuation_factors _att_factors,
		point_shadow_caster_properties _shadow_props = point_light::default_shadow_caster_properties
	);

	void prepare_draw(int index, const shader_program &shader, render_pass_state &render_pass) const override;
	void prepare_shadow_render_pass() const override;
	void prepare_draw_shadow_map(const shader_program &shader) const override;
	void set_casts_shadow(bool enabled) override;

	void set_pos(const glm::vec3 &_pos);
	const glm::vec3& get_pos() const;
	unsigned int get_depth_cubemap_id() const;
	const std::string& shadow_map_shader_name() const override;

protected:
	bool is_eq(const light &other) const override;

private:
	static const point_shadow_caster_properties default_shadow_caster_properties;

	unique_handle<unsigned int> depth_cubemap;
};

