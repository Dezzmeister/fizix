#extension GL_ARB_explicit_uniform_location : enable

layout(points) in;
layout(triangle_strip, max_vertices = 6) out;

flat out int ascii_val;
out vec2 uv;

layout(location = 1) uniform int x_offset;
layout(location = 2) uniform int y_offset;
layout(location = 3) uniform int glyph_width;
layout(location = 4) uniform int glyph_height;
layout(location = 5) uniform int screen_width;
layout(location = 6) uniform int screen_height;

void main() {
	float y0 = (2.0 * ((screen_height / 2.0) - y_offset)) / screen_height;
	float x0 = (2.0 * (x_offset + (gl_PrimitiveIDIn * glyph_width) - (screen_width / 2.0))) / screen_width;
	float xs = (2.0 * glyph_width) / screen_width;
	float ys = (2.0 * glyph_height) / screen_height;

	gl_Position = vec4(x0, y0, 0.0, 1.0);
	ascii_val = int(gl_in[0].gl_Position.x);
	uv = vec2(0.0, 0.0);
	EmitVertex();
	gl_Position = vec4(x0 + xs, y0, 0.0, 1.0);
	ascii_val = int(gl_in[0].gl_Position.x);
	uv = vec2(1.0, 0.0);
	EmitVertex();
	gl_Position = vec4(x0, y0 + ys, 0.0, 1.0);
	ascii_val = int(gl_in[0].gl_Position.x);
	uv = vec2(0.0, 1.0);
	EmitVertex();
	EndPrimitive();

	gl_Position = vec4(x0 + xs, y0, 0.0, 1.0);
	ascii_val = int(gl_in[0].gl_Position.x);
	uv = vec2(1.0, 0.0);
	EmitVertex();
	gl_Position = vec4(x0 + xs, y0 + ys, 0.0, 1.0);
	ascii_val = int(gl_in[0].gl_Position.x);
	uv = vec2(1.0, 1.0);
	EmitVertex();
	gl_Position = vec4(x0, y0 + ys, 0.0, 1.0);
	ascii_val = int(gl_in[0].gl_Position.x);
	uv = vec2(0.0, 1.0);
	EmitVertex();
	EndPrimitive();
}