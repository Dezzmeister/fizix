#define _USE_MATH_DEFINES
#include <math.h>
#include "shapes.h"

// Geometry is defined with a clockwise vertex winding order, even though backface culling
// is enabled (for the main color pass) and the frontface is counterclockwise. This is because
// the perspective matrix has a negative determinant, and so flips the winding order of vertices.

std::unique_ptr<geometry> shapes::cube{};
std::unique_ptr<geometry> shapes::plane{};

namespace {
	bool is_initialized{};
}

void shapes::init() {
	if (is_initialized) {
		return;
	}

	is_initialized = true;

	shapes::cube = std::make_unique<geometry>(std::vector<float>{
		// Front
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
		0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
		0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
		-0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,

		// Back
		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
		0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
		-0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,

		// Right
		-0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
		-0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		-0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		-0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,

		// Left
		0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
		0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
		0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

		// Top
		0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
		-0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,

		// Bottom
		-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
		-0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f
	});

	shapes::plane = std::make_unique<geometry>(std::vector<float>{
		0.5f, 0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		-0.5f, 0.0f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
		-0.5f, 0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,

		0.5f, 0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		0.5f, 0.0f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		-0.5f, 0.0f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
	});
}

static glm::vec3 spherical_to_cartesian(float r, float phi, float theta) {
	float r_sin_theta = r * std::sin(theta);

	return glm::vec3(
		r_sin_theta * std::cos(phi),
		r_sin_theta * std::sin(phi),
		r * std::cos(theta)
	);
}

static glm::vec2 spherical_to_uv(float phi, float theta) {
	return glm::vec2(
		phi / (M_PI * 2.0f),
		theta / M_PI
	);
}

static glm::vec3 cylindrical_to_cartesian(float r, float z, float phi) {
	// For our purposes, the "height" in cylindrical coordinates corresponds
	// to "y" in Cartesian coordinates, not "z"
	return glm::vec3(
		r * std::cos(phi),
		z,
		r * std::sin(phi)
	);
}

static glm::vec2 cylindrical_side_to_uv(float z, float phi, float h) {
	return glm::vec2(
		phi / (M_PI * 2.0f),
		(z / h) + 0.5f
	);
}

static glm::vec2 cylindrical_cap_to_uv(float z, float phi, float h) {
	if ((z / h) + 0.5f < 0.5f) {
		return glm::vec2(
			1.0f - std::cos(phi),
			std::sin(phi)
		);
	}

	return glm::vec2(
		std::cos(phi),
		std::sin(phi)
	);
}

static void insert(const glm::vec3 &v, std::vector<float> &out) {
	out.push_back(v.x);
	out.push_back(v.y);
	out.push_back(v.z);
}

static void insert(const glm::vec2 &v, std::vector<float> &out) {
	out.push_back(v.x);
	out.push_back(v.y);
}

geometry shapes::make_sphere(
	size_t horizontal_divisions,
	size_t vertical_divisions,
	bool smooth_normals
) {
	std::vector<float> out{};
	const float phi_step = (float)M_PI * 2.0f / (float)horizontal_divisions;
	const float theta_step = (float)M_PI / (float)vertical_divisions;
	const float r = 0.5f;

	for (float phi = 0.0f; phi < M_PI * 2.0f; phi += phi_step) {
		for (float theta = 0.0f; theta < M_PI; theta += theta_step) {
			// Vertex order of the the current tri pair (from the outside)
			//		v2  v0
			//		v3  v1

			glm::vec3 v0 = spherical_to_cartesian(r, phi, theta);
			glm::vec2 u0 = spherical_to_uv(phi, theta);

			glm::vec3 v1 = spherical_to_cartesian(r, phi, theta + theta_step);
			glm::vec2 u1 = spherical_to_uv(phi, theta + theta_step);

			glm::vec3 v2 = spherical_to_cartesian(r, phi + phi_step, theta);
			glm::vec2 u2 = spherical_to_uv(phi + phi_step, theta);

			glm::vec3 v3 = spherical_to_cartesian(r, phi + phi_step, theta + theta_step);
			glm::vec2 u3 = spherical_to_uv(phi + phi_step, theta + theta_step);

			glm::vec3 norm0 = glm::normalize(v0);
			glm::vec3 norm1 = glm::normalize(v1);
			glm::vec3 norm2 = glm::normalize(v2);
			glm::vec3 norm11 = norm1;
			glm::vec3 norm31 = glm::normalize(v3);
			glm::vec3 norm21 = norm2;

			if (! smooth_normals) {
				norm0 = glm::normalize(v0 + v1 + v2 + v3);
				norm1 = norm0;
				norm2 = norm0;
				norm11 = norm0;
				norm31 = norm0;
				norm21 = norm0;
			}

			// First triangle

			insert(v0, out);
			insert(norm0, out);
			insert(u0, out);

			insert(v1, out);
			insert(norm1, out);
			insert(u1, out);

			insert(v2, out);
			insert(norm2, out);
			insert(u2, out);

			// Second triangle

			insert(v1, out);
			insert(norm11, out);
			insert(u1, out);

			insert(v3, out);
			insert(norm31, out);
			insert(u3, out);

			insert(v2, out);
			insert(norm21, out);
			insert(u2, out);
		}
	}

	return geometry(out);
}

geometry shapes::make_cylinder(size_t divisions, bool smooth_normals) {
	std::vector<float> out{};
	const float r = 0.5f;
	const float h = 1.0f;
	const float z = h / 2.0f;
	const float phi_step = (float)M_PI * 2.0f / (float)divisions;

	glm::vec3 top = cylindrical_to_cartesian(0, z, 0);
	glm::vec2 top_uv = glm::vec2(0.5f, 0.5f);
	glm::vec3 top_norm = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 bot = cylindrical_to_cartesian(0, -z, 0);
	glm::vec2 bot_uv = glm::vec2(0.5f, 0.5f);
	glm::vec3 bot_norm = glm::vec3(0.0f, -1.0f, 0.0f);

	for (float phi = 0; phi < (float)M_PI * 2.0f; phi += phi_step) {
		// v0  v1
		// v2  v3
		glm::vec3 v0 = cylindrical_to_cartesian(r, z, phi);
		glm::vec3 v1 = cylindrical_to_cartesian(r, z, phi + phi_step);
		glm::vec3 v2 = cylindrical_to_cartesian(r, -z, phi);
		glm::vec3 v3 = cylindrical_to_cartesian(r, -z, phi + phi_step);

		glm::vec2 u0s = cylindrical_side_to_uv(z, phi, h);
		glm::vec2 u1s = cylindrical_side_to_uv(z, phi + phi_step, h);
		glm::vec2 u2s = cylindrical_side_to_uv(-z, phi, h);
		glm::vec2 u3s = cylindrical_side_to_uv(-z, phi + phi_step, h);

		glm::vec2 u0c = cylindrical_cap_to_uv(z, phi, h);
		glm::vec2 u1c = cylindrical_cap_to_uv(z, phi + phi_step, h);
		glm::vec2 u2c = cylindrical_cap_to_uv(-z, phi, h);
		glm::vec2 u3c = cylindrical_cap_to_uv(-z, phi + phi_step, h);

		glm::vec3 norm0;
		glm::vec3 norm2;
		glm::vec3 norm1;
		glm::vec3 norm21;
		glm::vec3 norm31;
		glm::vec3 norm11;

		if (smooth_normals) {
			norm0 = v0;
			norm0.y = 0.0f;
			norm0 = glm::normalize(norm0);
			norm2 = norm0;
			norm21 = norm0;

			norm1 = v1;
			norm1.y = 0.0f;
			norm1 = glm::normalize(norm1);
			norm31 = norm1;
			norm11 = norm1;
		} else {
			norm0 = glm::normalize(v0 + v1 + v2 + v3);
			norm2 = norm0;
			norm1 = norm0;
			norm21 = norm0;
			norm31 = norm0;
			norm11 = norm0;
		}

		// Top triangle

		insert(v0, out);
		insert(top_norm, out);
		insert(u0c, out);

		insert(top, out);
		insert(top_norm, out);
		insert(top_uv, out);

		insert(v1, out);
		insert(top_norm, out);
		insert(u1c, out);

		// Bottom triangle

		insert(v2, out);
		insert(bot_norm, out);
		insert(u2c, out);

		insert(v3, out);
		insert(bot_norm, out);
		insert(u3c, out);

		insert(bot, out);
		insert(bot_norm, out);
		insert(bot_uv, out);

		// First side triangle

		insert(v0, out);
		insert(norm0, out);
		insert(u0s, out);

		insert(v1, out);
		insert(norm1, out);
		insert(u1s, out);

		insert(v2, out);
		insert(norm2, out);
		insert(u2s, out);

		// Second side triangle

		insert(v3, out);
		insert(norm31, out);
		insert(u3s, out);

		insert(v2, out);
		insert(norm21, out);
		insert(u2s, out);

		insert(v1, out);
		insert(norm11, out);
		insert(u1s, out);
	}

	return geometry(out);
}