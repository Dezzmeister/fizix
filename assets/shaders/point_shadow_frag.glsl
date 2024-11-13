#extension GL_ARB_explicit_uniform_location : enable

in vec4 frag_pos;

layout(location = 8) uniform vec3 light_pos;
layout(location = 9) uniform float far_plane;

void main() {
	float light_dist = length(frag_pos.xyz - light_pos);

	light_dist = light_dist / far_plane;

	gl_FragDepth = light_dist;
}
