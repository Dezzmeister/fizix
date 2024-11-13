#pragma once
#include <Windows.h>
#include <cstdint>
#include <string>
#include <gl/gl.h>

#define GL_CALL								__stdcall

#define GL_FRAGMENT_SHADER					0x8B30
#define GL_VERTEX_SHADER					0x8B31
#define GL_GEOMETRY_SHADER					0x8DD9

#define GL_COMPILE_STATUS                   0x8B81
#define GL_LINK_STATUS						0x8B82

#define GL_FRAMEBUFFER						0x8D40
#define GL_DEPTH_ATTACHMENT                 0x8D00
#define GL_FRAMEBUFFER_COMPLETE             0x8CD5

#define GL_ARRAY_BUFFER                     0x8892
#define GL_STREAM_DRAW                      0x88E0
#define GL_STATIC_DRAW                      0x88E4

#define GL_TEXTURE0                         0x84C0
#define GL_TEXTURE1                         0x84C1
#define GL_TEXTURE2                         0x84C2
#define GL_TEXTURE3                         0x84C3
#define GL_TEXTURE4                         0x84C4
#define GL_TEXTURE5                         0x84C5
#define GL_TEXTURE6                         0x84C6
#define GL_TEXTURE7                         0x84C7
#define GL_TEXTURE8                         0x84C8
#define GL_TEXTURE9                         0x84C9
#define GL_TEXTURE10                        0x84CA
#define GL_TEXTURE11                        0x84CB
#define GL_TEXTURE12                        0x84CC
#define GL_TEXTURE13                        0x84CD
#define GL_TEXTURE14                        0x84CE
#define GL_TEXTURE15                        0x84CF
#define GL_TEXTURE16                        0x84D0
#define GL_TEXTURE17                        0x84D1
#define GL_TEXTURE18                        0x84D2
#define GL_TEXTURE19                        0x84D3
#define GL_TEXTURE20                        0x84D4
#define GL_TEXTURE21                        0x84D5
#define GL_TEXTURE22                        0x84D6
#define GL_TEXTURE23                        0x84D7
#define GL_TEXTURE24                        0x84D8
#define GL_TEXTURE25                        0x84D9
#define GL_TEXTURE26                        0x84DA
#define GL_TEXTURE27                        0x84DB
#define GL_TEXTURE28                        0x84DC
#define GL_TEXTURE29                        0x84DD
#define GL_TEXTURE30                        0x84DE
#define GL_TEXTURE31                        0x84DF
#define GL_ACTIVE_TEXTURE                   0x84E0

#define GL_CLAMP_TO_BORDER                  0x812D
#define GL_CLAMP_TO_EDGE                    0x812F

#define GL_MAX_TEXTURE_IMAGE_UNITS          0x8872

#define GL_TEXTURE_CUBE_MAP					0x8513
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X      0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X      0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y      0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y      0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z      0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z      0x851A

#define GL_TEXTURE_WRAP_R                   0x8072

#define GL_BGR                            0x80E0
#define GL_BGRA                           0x80E1

#define GL_VERTEX_PROGRAM_POINT_SIZE      0x8642

using GLchar = char;
using GLintptr = intptr_t;
using GLsizeiptr = uintptr_t;

extern GLuint (GL_CALL * glCreateShader)(GLenum type);
extern void (GL_CALL * glDeleteShader)(GLuint shader);
extern void (GL_CALL * glShaderSource)(GLuint shader, GLsizei count, const GLchar * const * string, const GLint * length);
extern void (GL_CALL * glCompileShader)(GLuint shader);
extern void (GL_CALL * glGetShaderiv)(GLuint shader, GLenum pname, GLint * params);
extern void (GL_CALL * glGetShaderInfoLog)(GLuint shader, GLsizei buf_size, GLsizei * length, GLchar * info_log);
extern GLuint (GL_CALL * glCreateProgram)();
extern void (GL_CALL * glDeleteProgram)(GLuint program);
extern void (GL_CALL * glAttachShader)(GLuint program, GLuint shader);
extern void (GL_CALL * glLinkProgram)(GLuint program);
extern void (GL_CALL * glGetProgramiv)(GLuint program, GLenum pname, GLint * params);
extern void (GL_CALL * glGetProgramInfoLog)(GLuint program, GLsizei buf_size, GLsizei * length, GLchar * info_log);
extern void (GL_CALL * glUseProgram)(GLuint program);
extern GLint (GL_CALL * glGetUniformLocation)(GLuint program, const GLchar * name);

extern void (GL_CALL * glUniform1f)(GLint location, GLfloat f0);
extern void (GL_CALL * glUniform3f)(GLint location, GLfloat f0, GLfloat f1, GLfloat f2);
extern void (GL_CALL * glUniform4f)(GLint location, GLfloat f0, GLfloat f1, GLfloat f2, GLfloat f3);
extern void (GL_CALL * glUniform1i)(GLint location, GLint i0);
extern void (GL_CALL * glUniform1ui)(GLint location, GLuint u0);
extern void (GL_CALL * glUniformMatrix3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
extern void (GL_CALL * glUniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);

extern void (GL_CALL * glGenFramebuffers)(GLsizei n, GLuint * framebuffers);
extern void (GL_CALL * glDeleteFramebuffers)(GLsizei n, const GLuint * framebuffers);
extern void (GL_CALL * glBindFramebuffer)(GLenum target, GLuint framebuffer);
extern void (GL_CALL * glFramebufferTexture2D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
extern GLenum (GL_CALL * glCheckFramebufferStatus)(GLenum target);
extern void (GL_CALL * glFramebufferTexture)(GLenum target, GLenum attachment, GLuint texture, GLint level);

extern void (GL_CALL * glActiveTexture)(GLenum texture);
extern void (GL_CALL * glGenerateMipmap)(GLenum target);

extern void (GL_CALL * glGenVertexArrays)(GLsizei n, GLuint * arrays);
extern void (GL_CALL * glDeleteVertexArrays)(GLsizei n, const GLuint * arrays);
extern void (GL_CALL * glBindVertexArray)(GLuint array);
extern void (GL_CALL * glGenBuffers)(GLsizei n, GLuint * buffers);
extern void (GL_CALL * glDeleteBuffers)(GLsizei n, const GLuint * buffers);
extern void (GL_CALL * glBindBuffer)(GLenum target, GLuint buffer);
extern void (GL_CALL * glVertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void * pointer);
extern void (GL_CALL * glVertexAttribIPointer)(GLuint index, GLint size, GLenum type, GLsizei stride, const void * pointer);
extern void (GL_CALL * glEnableVertexAttribArray)(GLuint index);
extern void (GL_CALL * glDisableVertexAttribArray)(GLuint index);
extern void (GL_CALL * glBufferData)(GLenum target, GLsizeiptr size, const void * data, GLenum usage);
extern void (GL_CALL * glBufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, void * data);
extern void (GL_CALL * glVertexAttribDivisor)(GLuint index, GLuint divisor);

extern void (GL_CALL * glDrawArraysInstanced)(GLenum mode, GLint first, GLsizei count, GLsizei instance_count);

void init_gl();
void * get_gl_func(const char * const func_name);
