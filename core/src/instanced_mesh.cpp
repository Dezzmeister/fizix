#include "instanced_mesh.h"

model_pair::model_pair(const glm::mat4 &_model, const glm::mat4 &_inv_model) :
	model(_model), inv_model(_inv_model)
{}

instanced_mesh::instanced_mesh(const geometry * _geom, const material * _mtl, size_t _instances) :
	geom(_geom),
	mtl(_mtl),
	models(_instances, model_pair(glm::identity<glm::mat4>(), glm::identity<glm::mat4>())),
	vbo(0, [](unsigned int _handle) {
		glDeleteBuffers(1, &_handle);
	}),
	models_need_updating(false)
{
	geom->prepare_draw();
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	// TODO: Parameterize buffer usage hint
	glBufferData(GL_ARRAY_BUFFER, sizeof(decltype(models)::value_type) * models.size(), models.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 32 * sizeof(float), (void*)0);
	glVertexAttribDivisor(3, 1);

	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 32 * sizeof(float), (void*)(4 * sizeof(float)));
	glVertexAttribDivisor(4, 1);

	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 32 * sizeof(float), (void*)(8 * sizeof(float)));
	glVertexAttribDivisor(5, 1);

	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 32 * sizeof(float), (void*)(12 * sizeof(float)));
	glVertexAttribDivisor(6, 1);

	glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 32 * sizeof(float), (void*)(16 * sizeof(float)));
	glVertexAttribDivisor(7, 1);

	glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, 32 * sizeof(float), (void*)(20 * sizeof(float)));
	glVertexAttribDivisor(8, 1);

	glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, 32 * sizeof(float), (void*)(24 * sizeof(float)));
	glVertexAttribDivisor(9, 1);

	glVertexAttribPointer(10, 4, GL_FLOAT, GL_FALSE, 32 * sizeof(float), (void*)(28 * sizeof(float)));
	glVertexAttribDivisor(10, 1);

	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	glEnableVertexAttribArray(5);
	glEnableVertexAttribArray(6);
	glEnableVertexAttribArray(7);
	glEnableVertexAttribArray(8);
	glEnableVertexAttribArray(9);
	glEnableVertexAttribArray(10);
}

void instanced_mesh::draw(draw_event&, const shader_program&) const {
	geom->prepare_draw();
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 32 * sizeof(float), (void*)0);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 32 * sizeof(float), (void*)(4 * sizeof(float)));
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 32 * sizeof(float), (void*)(8 * sizeof(float)));
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 32 * sizeof(float), (void*)(12 * sizeof(float)));
	glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 32 * sizeof(float), (void*)(16 * sizeof(float)));
	glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, 32 * sizeof(float), (void*)(20 * sizeof(float)));
	glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, 32 * sizeof(float), (void*)(24 * sizeof(float)));
	glVertexAttribPointer(10, 4, GL_FLOAT, GL_FALSE, 32 * sizeof(float), (void*)(28 * sizeof(float)));

	if (models_need_updating) {
		// GL_DYNAMIC_DRAW instead?
		glBufferData(GL_ARRAY_BUFFER, sizeof(decltype(models)::value_type) * models.size(), models.data(), GL_STATIC_DRAW);
		models_need_updating = false;
	}

	glDrawArraysInstanced(GL_TRIANGLES, 0, (GLsizei)geom->get_num_vertices(), (GLsizei)models.size());
}

void instanced_mesh::set_model(size_t i, const glm::mat4 &_model) {
	models[i].model = _model;
	models[i].inv_model = glm::inverse(_model);
	models_need_updating = true;
}

const glm::mat4& instanced_mesh::get_model(size_t i) const {
	return models[i].model;
}

bool operator==(const instanced_mesh &a, const instanced_mesh &b) {
	return a.vbo == b.vbo;
}
