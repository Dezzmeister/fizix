#pragma once
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include "gl.h"
#include "unique_handle.h"
#include "util.h"

enum class shader_type {
	Vertex,
	Geometry,
	Fragment
};

constexpr unsigned int get_gl_shader_type(shader_type type) {
	if (type == shader_type::Vertex) {
		return GL_VERTEX_SHADER;
	} else if (type == shader_type::Geometry) {
		return GL_GEOMETRY_SHADER;
	} else if (type == shader_type::Fragment) {
		return GL_FRAGMENT_SHADER;
	} else {
		// This shouldn't be possible
		throw std::invalid_argument("Unsupported shader type");
	}
}

template <shader_type ShaderType>
class shader {
public:
	shader(const char * const path, const std::string directives = "\n") :
		id(0, [](unsigned int _handle) {
		glDeleteShader(_handle);
			})
	{
		constexpr unsigned int gl_shader_type = get_gl_shader_type(ShaderType);

		std::string shader_code;
		std::ifstream shader_file;

		shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		try {
			shader_file.open(path);

			std::stringstream shader_stream;

			shader_stream << shader_file.rdbuf();
			shader_file.close();

			shader_code = shader_stream.str();
		} catch (std::ifstream::failure e) {
			std::cout << "Failed to read shader file: \"" << path << "\"" << std::endl;
			throw e;
		}

		int status;
		const char * const sources[] = {
			"#version 330 core\n",
			directives.c_str(),
			shader_code.c_str()
		};

		id = glCreateShader(gl_shader_type);
		glShaderSource(id, (GLsizei)util::c_arr_size(sources), sources, NULL);
		glCompileShader(id);
		glGetShaderiv(id, GL_COMPILE_STATUS, &status);

		if (!status) {
			char info_log[512];

			glGetShaderInfoLog(id, (GLsizei)util::c_arr_size(info_log), NULL, info_log);
			std::string type_str("!!!");

			if constexpr (ShaderType == shader_type::Vertex) {
				type_str = "vertex";
			} else if constexpr (ShaderType == shader_type::Geometry) {
				type_str = "geometry";
			} else if constexpr (ShaderType == shader_type::Fragment) {
				type_str = "fragment";
			}

			std::cout << "Failed to compile " << type_str << " shader (" << path << "): " << info_log << std::endl;
			// TODO: Proper error classes
			throw "Shader compilation error";
		}
	}

	unsigned int get_id() const {
		if (!id) {
			// TODO: Proper error classes
			throw "Error: shader has been moved";
		}

		return id;
	}

private:
	unique_handle<unsigned int> id;
};

