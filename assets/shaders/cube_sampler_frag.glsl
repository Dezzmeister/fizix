#extension GL_ARB_explicit_uniform_location : enable

in vec2 uv;

out vec4 frag_color;

layout(location = 8) uniform samplerCube cubemap;
layout(location = 9) uniform vec3 u_filter;
layout(location = 10) uniform vec3 v_filter;

void main() {
	vec2 pos = (uv * 2.0) - 1.0;
	vec3 index = (u_filter * vec3(pos.x, pos.x, pos.x)) + (v_filter * vec3(pos.y, pos.y, pos.y));
	float depth = texture(cubemap, index).r;

	frag_color = vec4(depth, depth, depth, 1.0);
}
