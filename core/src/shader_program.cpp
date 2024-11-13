#include "shader_program.h"

shader_program::shader_program(
	shader<shader_type::Vertex> vertex_shader,
	std::optional<shader<shader_type::Geometry>> geometry_shader,
	shader<shader_type::Fragment> fragment_shader
) :
	id(0, [](unsigned int _handle) {
	glDeleteProgram(_handle);
		})
{
	id = glCreateProgram();
	glAttachShader(id, vertex_shader.get_id());
	if (geometry_shader.has_value()) {
		glAttachShader(id, geometry_shader->get_id());
	}
	glAttachShader(id, fragment_shader.get_id());
	glLinkProgram(id);

	int status;

	glGetProgramiv(id, GL_LINK_STATUS, &status);

	if (!status) {
		char info_log[512];

		glGetProgramInfoLog(id, 512, NULL, info_log);
		std::cout << "Shader linking failed\n" << info_log << std::endl;

		// TODO: Proper errors
		throw "Shader linking failed";
	}
}

void shader_program::use() const {
	glUseProgram(id);
}

unsigned int shader_program::get_id() const {
	return id;
}

int shader_program::get_location(const std::string &name) const {
	auto loc = uniforms.find(name);

	if (loc != uniforms.end()) {
		return loc->second;
	}

	int found_loc = glGetUniformLocation(id, name.c_str());
	uniforms[name] = found_loc;

	return found_loc;
}

void shader_program::set_uniform(const std::string name, float value) const {
	const int loc = get_location(name);
	glUniform1f(loc, value);
}

void shader_program::set_uniform(const std::string name, int value) const {
	const int loc = get_location(name);
	glUniform1i(loc, value);
}

void shader_program::set_uniform(const std::string name, unsigned int value) const {
	const int loc = get_location(name);
	glUniform1ui(loc, value);
}

void shader_program::set_uniform(const std::string name, const bool value) const {
	const int loc = get_location(name);
	glUniform1i(loc, value ? 1 : 0);
}

void shader_program::set_uniform(const std::string name, const glm::mat4 &value) const {
	const int loc = get_location(name);
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
}

void shader_program::set_uniform(const std::string name, const glm::vec3 &value) const {
	const int loc = get_location(name);
	glUniform3f(loc, value.x, value.y, value.z);
}

void shader_program::set_uniform(const std::string name, const glm::mat3 &value) const {
	const int loc = get_location(name);
	glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(value));
}

void shader_program::set_uniform(const std::string name, const glm::vec4 &value) const {
	const int loc = get_location(name);
	glUniform4f(loc, value.x, value.y, value.z, value.w);
}

void shader_program::set_uniform(int loc, float value) const {
	glUniform1f(loc, value);
}

void shader_program::set_uniform(int loc, int value) const {
	glUniform1i(loc, value);
}

void shader_program::set_uniform(int loc, unsigned int value) const {
	glUniform1ui(loc, value);
}

void shader_program::set_uniform(int loc, bool value) const {
	glUniform1i(loc, value ? 1 : 0);
}

void shader_program::set_uniform(int loc, const glm::mat4 &value) const {
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
}

void shader_program::set_uniform(int loc, const glm::vec3 &value) const {
	glUniform3f(loc, value.x, value.y, value.z);
}

void shader_program::set_uniform(int loc, const glm::mat3 &value) const {
	glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(value));
}

void shader_program::set_uniform(int loc, const glm::vec4 &value) const {
	glUniform4f(loc, value.x, value.y, value.z, value.w);
}
