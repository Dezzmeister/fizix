#pragma once
#include <vector>
#include "material.h"

// A flat texture. An object with this material does not interact with lights.
class texture_material : public material {
public:
	const std::string texture_name;

	texture_material(std::string _texture_name);

	void prepare_draw(draw_event &event, const shader_program &shader, render_pass_state &render_pass) const override;
	bool supports_transparency() const override;
	const std::string& shader_name() const override;

private:
	const static std::string texture_shader_name;
};

