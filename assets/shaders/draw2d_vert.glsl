#extension GL_ARB_explicit_uniform_location : enable

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 uv_in;

out vec2 frag_uv;

void main() {
	frag_uv = uv_in;
	gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
}