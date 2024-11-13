#extension GL_ARB_explicit_uniform_location : enable

in vec2 frag_uv;

out vec4 frag_color;

layout(location = 11) uniform sampler2D icon;

void main() {
	frag_color = texture(icon, frag_uv);
}