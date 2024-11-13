#pragma once
#include "material.h"

struct phong_color_material_properties {
	glm::vec3 ambient{ 0.0f };
	glm::vec3 diffuse{ 0.0f };
	glm::vec3 specular{ 0.0f };
	float shininess{ 0 };

	constexpr phong_color_material_properties() = default;

	constexpr phong_color_material_properties(
		glm::vec3 _ambient,
		glm::vec3 _diffuse,
		glm::vec3 _specular,
		float _shininess
	) :
		ambient(_ambient),
		diffuse(_diffuse),
		specular(_specular),
		shininess(_shininess)
	{}
};

class phong_color_material : public material {
public:
	constexpr phong_color_material(phong_color_material_properties _mat) :
		mat(_mat)
	{}

	void prepare_draw(draw_event &event, const shader_program &shader, render_pass_state &render_pass) const override;
	bool supports_transparency() const override;
	const std::string& shader_name() const override;

private:
	const static std::string phong_shader_name;

	const phong_color_material_properties mat;
};

