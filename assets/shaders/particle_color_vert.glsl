#extension GL_ARB_explicit_uniform_location : enable

layout(location = 0) in vec3 pos;
layout(location = 1) in vec4 color;

out vec4 vert_color;

layout(location = 1) uniform float max_particle_size;
layout(location = 21) uniform mat4 view;
layout(location = 22) uniform mat4 projection;

float compute_point_size(vec4 out_pos) {
	float dist = out_pos.z / out_pos.w;

	if (dist * max_particle_size <= 1) {
		return max_particle_size;
	}

	return 1.0 / dist;
}

void main() {
	vec4 out_pos = projection * view * vec4(pos, 1.0);
	gl_Position = out_pos;
	gl_PointSize = compute_point_size(out_pos);
	vert_color = color;
}