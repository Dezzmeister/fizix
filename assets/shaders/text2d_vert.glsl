layout(location = 0) in int ascii_val;

void main() {
	gl_Position = vec4(ascii_val, 0.0, 0.0, 1.0);
}