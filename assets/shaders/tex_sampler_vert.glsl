layout(location = 2) in vec2 tex_coords_in;

out vec2 uv;

void main() {
	vec2 norm = (tex_coords_in * 2) - vec2(1.0, 1.0);
	uv = tex_coords_in;
	gl_Position = vec4(norm.xy, -1.0, 1.0);
}
