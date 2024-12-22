#include <string>
#include "gl.h"
#include "logging.h"

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

void load_gl_funcs() {
	load_gl(glCreateShader);
	load_gl(glDeleteShader);
	load_gl(glShaderSource);
	load_gl(glCompileShader);
	load_gl(glGetShaderiv);
	load_gl(glGetShaderInfoLog);
	load_gl(glCreateProgram);
	load_gl(glDeleteProgram);
	load_gl(glAttachShader);
	load_gl(glLinkProgram);
	load_gl(glGetProgramiv);
	load_gl(glGetProgramInfoLog);
	load_gl(glUseProgram);
	load_gl(glGetUniformLocation);

	load_gl(glUniform1f);
	load_gl(glUniform3f);
	load_gl(glUniform4f);
	load_gl(glUniform1i);
	load_gl(glUniform1ui);
	load_gl(glUniformMatrix3fv);
	load_gl(glUniformMatrix4fv);

	load_gl(glGenFramebuffers);
	load_gl(glDeleteFramebuffers);
	load_gl(glBindFramebuffer);
	load_gl(glFramebufferTexture2D);
	load_gl(glCheckFramebufferStatus);
	load_gl(glFramebufferTexture);

	load_gl(glActiveTexture);
	load_gl(glGenerateMipmap);

	load_gl(glGenVertexArrays);
	load_gl(glDeleteVertexArrays);
	load_gl(glBindVertexArray);
	load_gl(glGenBuffers);
	load_gl(glDeleteBuffers);
	load_gl(glBindBuffer);
	load_gl(glVertexAttribPointer);
	load_gl(glVertexAttribIPointer);
	load_gl(glEnableVertexAttribArray);
	load_gl(glDisableVertexAttribArray);
	load_gl(glBufferData);
	load_gl(glBufferSubData);
	load_gl(glVertexAttribDivisor);

	load_gl(glDrawArraysInstanced);
}