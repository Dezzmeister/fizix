#extension GL_ARB_explicit_uniform_location : enable

out vec4 frag_color;

layout(location = 10) uniform vec4 color;

void main() {
	frag_color = color;
}
