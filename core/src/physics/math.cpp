#include <glm/gtx/quaternion.hpp>
#include "physics/math.h"

phys::real phys::dot(const vec3 &a, const vec3 &b) {
	return glm::dot(a, b);
}

phys::vec3 phys::cross(const vec3 &a, const vec3 &b) {
	return glm::cross(a, b);
}

phys::vec3 phys::normalize(const vec3 &v) {
	return glm::normalize(v);
}

phys::quat phys::normalize(const quat &q) {
	return glm::normalize(q);
}

phys::mat4 phys::transpose(const mat4 &m) {
	return glm::transpose(m);
}

phys::mat4 phys::inverse(const mat4 &m) {
	return glm::inverse(m);
}

phys::mat3 phys::inverse(const mat3 &m) {
	return glm::inverse(m);
}

phys::decomposed_vec3 phys::decompose_vec3(const vec3 &v, const vec3 &n) {
	decomposed_vec3 out{};
	real parallel_mag = phys::dot(v, n);

	out.parallel = parallel_mag * n;
	out.perp = v - out.parallel;

	return out;
}

phys::vec4 phys::transform_inverse(const mat4 &m, const vec4 &v) {
	const vec4 rot = v - m[3];
	const mat4 m_no_trans{
		m[0],
		m[1],
		m[2],
		vec4(0, 0, 0, 1)
	};

	return transpose(m_no_trans) * rot;
}

phys::quat phys::rotate_by(const quat &q, const vec3 &v) {
	const quat vq(0, v.x, v.y, v.z);

	return q * vq;
}

phys::vec3 phys::truncate(const vec4 &v) {
	return vec3(v.x, v.y, v.z);
}

phys::mat3 phys::truncate(const mat4 &m) {
	return mat3(
		truncate(m[0]),
		truncate(m[1]),
		truncate(m[2])
	);
}

phys::mat4 phys::quat_to_mat4(const phys::quat &q) {
	return glm::toMat4(q);
}

phys::mat4 phys::translate(const phys::vec3 &v) {
	return glm::translate(glm::identity<glm::mat4>(), v);
}