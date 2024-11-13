#include <algorithm>
#include "geometry.h"
#include "gl.h"

namespace {
	void insert(const glm::vec3 &v, std::vector<float> &out) {
		out.push_back(v.x);
		out.push_back(v.y);
		out.push_back(v.z);
	}

	std::vector<float> compute_tangent_basis(const std::vector<float> attrs) {
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
}

geometry::geometry(std::vector<float> _vbo_data) :
	num_vertices(_vbo_data.size() / 8),
	vbo_data(compute_tangent_basis(_vbo_data)),
	vao(0, [](unsigned int handle) {
	glDeleteVertexArrays(1, &handle);
		}),
	vbo(0, [](unsigned int handle) {
	glDeleteBuffers(1, &handle);
		})
{
	constexpr size_t stride = sizeof(float) * (3 + 3 + 2 + 3 + 3);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vbo_data.size() * sizeof(float), vbo_data.data(), GL_STATIC_DRAW);

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
}

void geometry::draw(int, unsigned int count) const {
	glDrawArrays(GL_TRIANGLES, 0, count == -1 ? (GLsizei)num_vertices : count);
}
