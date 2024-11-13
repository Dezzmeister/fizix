#include "traits.h"

template <>
std::string traits::to_string<glm::vec3>(const glm::vec3 &v, size_t) {
	return "vec3(" +
		to_string(v.x) + ", " +
		to_string(v.y) + ", " +
		to_string(v.z) + ")";
}

template <>
std::string traits::to_string<glm::vec4>(const glm::vec4 &v, size_t) {
	return "vec4(" +
		to_string(v.x) + ", " +
		to_string(v.y) + ", " +
		to_string(v.z) + ", " +
		to_string(v.w) + ")";
}

template <>
std::string traits::to_string<glm::quat>(const glm::quat &q, size_t) {
	return "quat(" +
		to_string(q.w) + ", [" +
		to_string(q.x) + ", " +
		to_string(q.y) + ", " +
		to_string(q.z) + "])";
}

template <>
std::string traits::to_string<glm::mat4>(const glm::mat4 &m, size_t indent) {
	return "mat4[" +
		std::string(indent, '\t') + to_string(m[0]) + ",\n" +
		std::string(indent, '\t') + to_string(m[1]) + ",\n" +
		std::string(indent, '\t') + to_string(m[2]) + ",\n" +
		std::string(indent, '\t') + to_string(m[3]) + "]";
}
