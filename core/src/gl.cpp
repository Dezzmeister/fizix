#include <string>
#include "gl.h"

#define load(func) func = (decltype(func)) get_gl_func(#func)

GLuint (GL_CALL * glCreateShader)(GLenum type);
void (GL_CALL * glDeleteShader)(GLuint shader);
void (GL_CALL * glShaderSource)(GLuint shader, GLsizei count, const GLchar * const * string, const GLint * length);
void (GL_CALL * glCompileShader)(GLuint shader);
void (GL_CALL * glGetShaderiv)(GLuint shader, GLenum pname, GLint * params);
void (GL_CALL * glGetShaderInfoLog)(GLuint shader, GLsizei buf_size, GLsizei * length, GLchar * info_log);
GLuint (GL_CALL * glCreateProgram)();
void (GL_CALL * glDeleteProgram)(GLuint program);
void (GL_CALL * glAttachShader)(GLuint program, GLuint shader);
void (GL_CALL * glLinkProgram)(GLuint program);
void (GL_CALL * glGetProgramiv)(GLuint program, GLenum pname, GLint * params);
void (GL_CALL * glGetProgramInfoLog)(GLuint program, GLsizei buf_size, GLsizei * length, GLchar * info_log);
void (GL_CALL * glUseProgram)(GLuint program);
GLint (GL_CALL * glGetUniformLocation)(GLuint program, const GLchar * name);

void (GL_CALL * glUniform1f)(GLint location, GLfloat f0);
void (GL_CALL * glUniform3f)(GLint location, GLfloat f0, GLfloat f1, GLfloat f2);
void (GL_CALL * glUniform4f)(GLint location, GLfloat f0, GLfloat f1, GLfloat f2, GLfloat f3);
void (GL_CALL * glUniform1i)(GLint location, GLint i0);
void (GL_CALL * glUniform1ui)(GLint location, GLuint u0);
void (GL_CALL * glUniformMatrix3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
void (GL_CALL * glUniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);

void (GL_CALL * glGenFramebuffers)(GLsizei n, GLuint * framebuffers);
void (GL_CALL * glDeleteFramebuffers)(GLsizei n, const GLuint * framebuffers);
void (GL_CALL * glBindFramebuffer)(GLenum target, GLuint framebuffer);
void (GL_CALL * glFramebufferTexture2D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
GLenum (GL_CALL * glCheckFramebufferStatus)(GLenum target);
void (GL_CALL * glFramebufferTexture)(GLenum target, GLenum attachment, GLuint texture, GLint level);

void (GL_CALL * glActiveTexture)(GLenum texture);
void (GL_CALL * glGenerateMipmap)(GLenum target);

void (GL_CALL * glGenVertexArrays)(GLsizei n, GLuint * arrays);
void (GL_CALL * glDeleteVertexArrays)(GLsizei n, const GLuint * arrays);
void (GL_CALL * glBindVertexArray)(GLuint array);
void (GL_CALL * glGenBuffers)(GLsizei n, GLuint * buffers);
void (GL_CALL * glDeleteBuffers)(GLsizei n, const GLuint * buffers);
void (GL_CALL * glBindBuffer)(GLenum target, GLuint buffer);
void (GL_CALL * glVertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void * pointer);
void (GL_CALL * glVertexAttribIPointer)(GLuint index, GLint size, GLenum type, GLsizei stride, const void * pointer);
void (GL_CALL * glEnableVertexAttribArray)(GLuint index);
void (GL_CALL * glDisableVertexAttribArray)(GLuint index);
void (GL_CALL * glBufferData)(GLenum target, GLsizeiptr size, const void * data, GLenum usage);
void (GL_CALL * glBufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, void * data);
void (GL_CALL * glVertexAttribDivisor)(GLuint index, GLuint divisor);

void (GL_CALL * glDrawArraysInstanced)(GLenum mode, GLint first, GLsizei count, GLsizei instance_count);

void init_gl() {
	load(glCreateShader);
	load(glDeleteShader);
	load(glShaderSource);
	load(glCompileShader);
	load(glGetShaderiv);
	load(glGetShaderInfoLog);
	load(glCreateProgram);
	load(glDeleteProgram);
	load(glAttachShader);
	load(glLinkProgram);
	load(glGetProgramiv);
	load(glGetProgramInfoLog);
	load(glUseProgram);
	load(glGetUniformLocation);

	load(glUniform1f);
	load(glUniform3f);
	load(glUniform4f);
	load(glUniform1i);
	load(glUniform1ui);
	load(glUniformMatrix3fv);
	load(glUniformMatrix4fv);

	load(glGenFramebuffers);
	load(glDeleteFramebuffers);
	load(glBindFramebuffer);
	load(glFramebufferTexture2D);
	load(glCheckFramebufferStatus);
	load(glFramebufferTexture);

	load(glActiveTexture);
	load(glGenerateMipmap);

	load(glGenVertexArrays);
	load(glDeleteVertexArrays);
	load(glBindVertexArray);
	load(glGenBuffers);
	load(glDeleteBuffers);
	load(glBindBuffer);
	load(glVertexAttribPointer);
	load(glVertexAttribIPointer);
	load(glEnableVertexAttribArray);
	load(glDisableVertexAttribArray);
	load(glBufferData);
	load(glBufferSubData);
	load(glVertexAttribDivisor);

	load(glDrawArraysInstanced);
}

void * get_gl_func(const char * const func_name) {
	void * out = (void *)wglGetProcAddress(func_name);

	if (! out || (out == (void *)1) || (out == (void *)2) || (out == (void *)3) || (out == (void *)-1)) {
		DWORD err = GetLastError();

		// TODO: Proper errors
		throw "Failed to load " + std::string(func_name) + ": Error code " + std::to_string(err);
	}

	return out;
}