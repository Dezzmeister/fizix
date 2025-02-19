#pragma once
#include "material.h"

// A flat color. An object with this material does not interact with lights.
class color_material : public material {
public:
	const glm::vec3 color;

	color_material(const glm::vec3 &_color);

	void prepare_draw(draw_event &event, const shader_program &shader, render_pass_state &render_pass) const override;
	bool supports_transparency() const override;
	const std::string& shader_name() const override;

private:
	const static std::string color_shader_name;
};

