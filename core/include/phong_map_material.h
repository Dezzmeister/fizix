#pragma once
#include "material.h"
#include "texture.h"

class phong_map_material : public material {
public:
	const std::string diffuse_map_name;
	const std::string specular_map_name;
	const std::string normal_map_name;
	float shininess;

	phong_map_material(
		const std::string &_diffuse_map_name,
		const std::string &_specular_map_name,
		const std::string &_normal_map_name,
		float _shininess
	);

	void prepare_draw(draw_event &event, const shader_program &shader, render_pass_state &render_pass) const override;
	bool supports_transparency() const override;
	const std::string& shader_name() const override;

private:
	const static std::string phong_map_shader_name;
};
