#extension GL_ARB_explicit_uniform_location : enable

flat in int ascii_val;
in vec2 uv;

out vec4 frag_color;

layout(location = 3) uniform int glyph_width;
layout(location = 7) uniform sampler2D font;
layout(location = 8) uniform vec4 bg_color;
layout(location = 9) uniform vec4 fg_color;

void main() {
	ivec2 tex_size = textureSize(font, 0);
	float x = ((uv.x + ascii_val - 32.0) * glyph_width / tex_size.x);
	float y = uv.y;

	vec4 p = texture(font, vec2(x, y));

	if (p.x == 1.0) {
		frag_color = fg_color;
	} else {
		frag_color = bg_color;
	}
}