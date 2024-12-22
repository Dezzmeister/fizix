#extension GL_ARB_explicit_uniform_location : enable
layout (location = 0) in vec3 pos;

// TODO: Multiply matrices ahead of time and use one uniform
layout(location = 20) uniform mat4 model;
layout(location = 21) uniform mat4 view;
layout(location = 22) uniform mat4 projection;

void main() {
	gl_Position = projection * view * model * vec4(pos, 1.0);
}