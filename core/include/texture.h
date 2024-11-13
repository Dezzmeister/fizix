#pragma once
#include <glm/glm.hpp>
#include "unique_handle.h"

class texture {
public:
	texture(const char * const path, bool generate_mipmap = true);

	unsigned int get_id() const;
	const glm::uvec2& get_dimensions() const;

	friend bool operator==(const texture &a, const texture &b);

private:
	unique_handle<unsigned int> id;
	glm::uvec2 dimensions{};
};
