#extension GL_ARB_explicit_uniform_location : enable

in vec2 uv;

out vec4 frag_color;

layout(location = 8) uniform sampler2D tex;

void main() {
	float depth = texture(tex, uv).r;

	frag_color = vec4(depth, depth, depth, 1.0);
}
