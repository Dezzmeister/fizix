#include "mesh.h"
#include "shader_constants.h"
#include "shader_store.h"
#include "util.h"

mesh::mesh(const geometry * _geom, const material * _mat, int _first, unsigned int _count) :
	model(glm::identity<glm::mat4>()),
	inv_model(glm::inverse(model)),
	geom(_geom),
	mat(_mat),
	first(_first),
	count(_count),
	alpha(1.0f)
{}

void mesh::prepare_draw(draw_event &event, const shader_program &shader, bool include_normal) const {
	static constexpr int model_loc = util::find_in_map(constants::shader_locs, "model");
	static constexpr int normal_mat_loc = util::find_in_map(constants::shader_locs, "normal_mat");
	static constexpr int alpha_loc = util::find_in_map(constants::shader_locs, "alpha");

	shader.set_uniform(model_loc, model);

	if (include_normal) {
		glm::mat3 normal_mat = glm::mat3(glm::transpose(inv_model * *event.inv_view));
		shader.set_uniform(normal_mat_loc, normal_mat);
	}

	if (has_transparency()) {
		shader.set_uniform(alpha_loc, alpha);
	}
}

void mesh::draw() const {
	geom->draw(first, count);
}

void mesh::set_model(const glm::mat4 &_model) {
	model = _model;
	inv_model = glm::inverse(model);
}

void mesh::set_alpha(float _alpha) {
	if (! mat->supports_transparency()) {
		// TODO: Throw an error or otherwise indicate to the caller that we can't set
		// transparency
		return;
	}

	alpha = std::clamp(_alpha, 0.0f, 1.0f);
}

const glm::mat4& mesh::get_model() const {
	return model;
}

const material * mesh::get_material() const {
	return mat;
}

bool mesh::has_transparency() const {
	return alpha != 1.0f;
}

bool operator<(const mesh &a, const mesh &b) {
	return std::make_tuple(
		a.mat, a.geom,
		a.model[0].x, a.model[0].y, a.model[0].z, a.model[0].w,
		a.model[1].x, a.model[1].y, a.model[1].z, a.model[1].w,
		a.model[2].x, a.model[2].y, a.model[2].z, a.model[2].w,
		a.model[3].x, a.model[3].y, a.model[3].z, a.model[3].w
	) < std::make_tuple(
		b.mat, b.geom,
		b.model[0].x, b.model[0].y, b.model[0].z, b.model[0].w,
		b.model[1].x, b.model[1].y, b.model[1].z, b.model[1].w,
		b.model[2].x, b.model[2].y, b.model[2].z, b.model[2].w,
		b.model[3].x, b.model[3].y, b.model[3].z, b.model[3].w
	);
}

bool operator==(const mesh &a, const mesh &b) {
	return (a.geom == b.geom) && (a.mat == b.mat) && (a.model == b.model);
}
