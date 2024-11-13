#extension GL_ARB_explicit_uniform_location : enable

layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

layout(location = 24) uniform mat4 view_proj[6];

out vec4 frag_pos;

void main() {
	for (int face = 0; face < 6; face++) {
		gl_Layer = face;

		for (int i = 0; i < 3; i++) {
			frag_pos = gl_in[i].gl_Position;
			gl_Position = view_proj[face] * frag_pos;
			EmitVertex();
		}
		EndPrimitive();
	}
}
