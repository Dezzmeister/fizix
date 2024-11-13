#pragma once
#include "light.h"

class spotlight : public light {
public:
	glm::vec3 pos;
	glm::vec3 dir;
	float cos_inner_cutoff;
	float cos_outer_cutoff;
	light_properties props;
	attenuation_factors att_factors;

	spotlight(
		const glm::vec3 _pos,
		const glm::vec3 _dir,
		float _inner_cutoff,
		float _outer_cutoff,
		const light_properties _props,
		const attenuation_factors _att_factors
	);

	void prepare_draw(int index, const shader_program &shader, render_pass_state &render_pass) const override;
	void prepare_shadow_render_pass() const override;
	void prepare_draw_shadow_map(const shader_program &shader) const override;

	void set_casts_shadow(bool enabled) override;

	const std::string& shadow_map_shader_name() const override;

protected:
	bool is_eq(const light &other) const override;
};
