layout(location = 0) in vec3 pos;
layout(location = 2) in vec2 tex_coord_in;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 tex_coord;

void main() {
	gl_Position = projection * view * model * vec4(pos, 1.0);
	tex_coord = tex_coord_in;
}