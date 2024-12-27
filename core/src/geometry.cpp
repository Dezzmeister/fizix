#include <algorithm>
#include "geometry.h"
#include "gl.h"

namespace {
	void insert(const glm::vec3 &v, std::vector<float> &out) {
		out.push_back(v.x);
		out.push_back(v.y);
		out.push_back(v.z);
	}

	void insert(const glm::vec2 &v, std::vector<float> &out) {
		out.push_back(v.x);
		out.push_back(v.y);
	}

	std::vector<float> compute_tangent_basis_for_triangles(
		const std::vector<float> &attrs
	) {
		constexpr size_t stride = 3 + 3 + 2;
		constexpr size_t uv_offset = 3 + 3;

		std::vector<float> out{};

		for (size_t i = 0; i < attrs.size(); i += (stride * 3)) {
			size_t offset1 = i;
			size_t offset2 = offset1 + stride;
			size_t offset3 = offset2 + stride;

			glm::vec3 p1 = glm::vec3(attrs[offset1], attrs[offset1 + 1], attrs[offset1 + 2]);
			glm::vec3 p2 = glm::vec3(attrs[offset2], attrs[offset2 + 1], attrs[offset2 + 2]);
			glm::vec3 p3 = glm::vec3(attrs[offset3], attrs[offset3 + 1], attrs[offset3 + 2]);
			glm::vec2 uv1 = glm::vec2(attrs[offset1 + uv_offset], attrs[offset1 + uv_offset + 1]);
			glm::vec2 uv2 = glm::vec2(attrs[offset2 + uv_offset], attrs[offset2 + uv_offset + 1]);
			glm::vec2 uv3 = glm::vec2(attrs[offset3 + uv_offset], attrs[offset3 + uv_offset + 1]);

			glm::vec3 e1 = p2 - p1;
			glm::vec3 e2 = p3 - p1;
			float du1 = uv2.x - uv1.x;
			float dv1 = uv2.y - uv1.y;
			float du2 = uv3.x - uv1.x;
			float dv2 = uv3.y - uv1.y;

			glm::mat2 duv{
				du1, dv1,
				du2, dv2
			};
			glm::mat2x3 e{
				e1,
				e2
			};

			glm::mat2x3 tb = e * glm::inverse(duv);
			glm::vec3 t = tb[0];
			glm::vec3 bt = tb[1];

			std::copy(std::begin(attrs) + offset1, std::begin(attrs) + offset2, std::back_inserter(out));
			insert(t, out);
			insert(bt, out);

			std::copy(std::begin(attrs) + offset2, std::begin(attrs) + offset3, std::back_inserter(out));
			insert(t, out);
			insert(bt, out);

			std::copy(std::begin(attrs) + offset3, std::begin(attrs) + offset3 + stride, std::back_inserter(out));
			insert(t, out);
			insert(bt, out);
		}

		return out;
	}

	std::vector<float> compute_empty_tangent_basis(
		const std::vector<float> &attrs
	) {
		constexpr size_t stride = 3 + 3 + 2;

		std::vector<float> out{};

		for (size_t i = 0; i < attrs.size(); i += stride) {
			std::copy(std::begin(attrs) + i, std::begin(attrs) + i + stride, std::back_inserter(out));
			insert(glm::vec3(0.0f), out);
			insert(glm::vec3(0.0f), out);
		}

		return out;
	}

	std::vector<float> compute_tangent_basis(
		const std::vector<float> &attrs,
		geometry_primitive_type primitive_type
	) {
		switch (primitive_type) {
			case geometry_primitive_type::Points:
			case geometry_primitive_type::Lines:
				return compute_empty_tangent_basis(attrs);
			case geometry_primitive_type::Triangles:
				return compute_tangent_basis_for_triangles(attrs);
			default:
				std::unreachable();
		}
	}
}

geometry::geometry(
	const std::vector<float> &_vbo_data,
	geometry_primitive_type _primitive_type,
	vbo_usage_hint _vbo_hint
) :
	num_vertices(_vbo_data.size() / 8),
	vbo_data(compute_tangent_basis(_vbo_data, _primitive_type)),
	vao(0, [](unsigned int handle) {
		glDeleteVertexArrays(1, &handle);
	}),
	vbo(0, [](unsigned int handle) {
		glDeleteBuffers(1, &handle);
	}),
	primitive_type(_primitive_type),
	vbo_hint(_vbo_hint)
{
	constexpr size_t stride = sizeof(float) * (3 + 3 + 2 + 3 + 3);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vbo_data.size() * sizeof(float), vbo_data.data(), static_cast<GLenum>(vbo_hint));

	// Vertices are always (location = 0)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);

	// Normals are always (location = 1)
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// UVs are always (location = 2)
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// Tangent vectors are (location = 3)
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(8 * sizeof(float)));
	glEnableVertexAttribArray(3);

	// Bitangent vectors are (location = 4)
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, stride, (void*)(11 * sizeof(float)));
	glEnableVertexAttribArray(4);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void geometry::prepare_draw() const {
	glBindVertexArray(vao);

	if (vbo_needs_update) {
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		// TODO: Use glBufferSubData instead of invalidating the entire vbo
		glBufferData(GL_ARRAY_BUFFER, vbo_data.size() * sizeof(float), vbo_data.data(), static_cast<GLenum>(vbo_hint));
		vbo_needs_update = false;
	}
}

void geometry::draw(int, int count) const {
	glDrawArrays(static_cast<GLenum>(primitive_type), 0, count == -1 ? (GLsizei)num_vertices : count);
}

size_t geometry::add_vertex(
	const glm::vec3 &pos,
	const glm::vec3 &norm,
	const glm::vec2 &uv,
	const glm::vec3 &tangent,
	const glm::vec3 &bitangent
) {
	insert(pos, vbo_data);
	insert(norm, vbo_data);
	insert(uv, vbo_data);
	insert(tangent, vbo_data);
	insert(bitangent, vbo_data);

	size_t out = num_vertices;
	num_vertices++;

	vbo_needs_update = true;

	return out;
}

void geometry::remove_vertex(size_t vertex_idx) {
	// TODO: Consolidate strides
	constexpr size_t stride = (3 + 3 + 2 + 3 + 3);

	vbo_data.erase(std::begin(vbo_data) + vertex_idx * stride, std::begin(vbo_data) + (vertex_idx + 1) * stride);

	assert(num_vertices != 0);
	num_vertices--;
	vbo_needs_update = true;
}

void geometry::clear_vertices() {
	vbo_data.clear();
	num_vertices = 0;
	vbo_needs_update = true;
}

size_t geometry::get_num_vertices() const {
	return num_vertices;
}

void geometry::write_vertex(
	std::vector<float> &out,
	const glm::vec3 &pos,
	const glm::vec3 &norm,
	const glm::vec2 &uv
) {
	insert(pos, out);
	insert(norm, out);
	insert(uv, out);
}
