#pragma once
#include <memory>
#include "events.h"
#include "unique_handle.h"

class geometry {
public:
	const size_t num_vertices;

	// Vertices, normals, and UVs are interleaved
	geometry(std::vector<float> _vbo_data);

	void prepare_draw() const;

	void draw(int first, unsigned int count) const;

private:
	std::vector<float> vbo_data;
	unique_handle<unsigned int> vao;
	unique_handle<unsigned int> vbo;
};

