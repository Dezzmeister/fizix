#pragma once
#include <vector>
#include "geometry.h"
#include "material.h"
#include "unique_handle.h"

class world;

// I don't know on what kind of system this struct would not be tightly packed
// already, but better to be sure
#pragma pack(push, 1)
struct model_pair {
	glm::mat4 model;
	glm::mat4 inv_model;

	model_pair(const glm::mat4 &_model, const glm::mat4 &_inv_model);
};
#pragma pack(pop)
// The vertex shaders expect two matrices, each divided up into 4
// quadruples of floats. `instanced_mesh` will send an array of these structs to
// the GPU, so we need to make sure that they have the correct size.
static_assert(sizeof(model_pair) == 16 * sizeof(float) * 2);

class instanced_mesh {
public:

	instanced_mesh(const geometry * _geom, const material * _mtl, size_t _instances);

	void draw(draw_event &event, const shader_program &shader) const;

	void set_model(size_t index, const glm::mat4 &_model);

	const glm::mat4& get_model(size_t index) const;

	friend bool operator==(const instanced_mesh &a, const instanced_mesh &b);

	friend class world;

private:
	const geometry * geom;
	const material * mtl;
	std::vector<model_pair> models;
	unique_handle<unsigned int> vbo;
	mutable bool models_need_updating;
};
