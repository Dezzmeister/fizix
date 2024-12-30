#pragma once
#include <memory>
#include "events.h"
#include "unique_handle.h"

enum class geometry_primitive_type {
	Points = GL_POINTS,
	Lines = GL_LINES,
	Triangles = GL_TRIANGLES,
};

enum class vbo_usage_hint {
	StaticDraw = GL_STATIC_DRAW,
	StreamDraw = GL_STREAM_DRAW,
	DynamicDraw = GL_STATIC_DRAW
};

// TODO: Make vbo_data a vector of these rather than plain floats
struct vbo_entry {
	float vertex[3]{};
	float normal[3]{};
	float uv[2]{};
	float tangent[3]{};
	float bitangent[3]{};
};

class geometry {
public:
	// Vertices, normals, and UVs are interleaved in the input buffer.
	// Bitangent and tangent vectors are computed per-triangle iff the
	// primitive type is Triangles. Otherwise, the bitangent and tangent
	// vectors will be zero.
	geometry(
		const std::vector<float> &_vbo_data,
		geometry_primitive_type _primitive_type = geometry_primitive_type::Triangles,
		vbo_usage_hint _vbo_hint = vbo_usage_hint::StaticDraw
	);

	void prepare_draw() const;
	void draw(int first, int count) const;

	size_t add_vertex(
		const glm::vec3 &pos,
		const glm::vec3 &norm,
		const glm::vec2 &uv,
		const glm::vec3 &tangent = glm::vec3(0.0f),
		const glm::vec3 &bitangent = glm::vec3(0.0f)
	);
	void remove_vertex(size_t vertex_idx);
	vbo_entry * get_vertex(size_t vertex_idx);
	const vbo_entry * get_vertex(size_t vertex_idx) const;
	void clear_vertices();
	size_t get_num_vertices() const;

	static void write_vertex(
		std::vector<float> &vbo_buf,
		const glm::vec3 &pos,
		const glm::vec3 &norm,
		const glm::vec2 &uv
	);

private:
	size_t num_vertices;
	std::vector<float> vbo_data;
	unique_handle<unsigned int> vao;
	unique_handle<unsigned int> vbo;
	geometry_primitive_type primitive_type;
	vbo_usage_hint vbo_hint;
	mutable bool vbo_needs_update{};
};

