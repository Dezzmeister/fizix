layout(location = 0) in int ascii_val;

flat out int ascii_val_geom;

void main() {
	gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
	ascii_val_geom = ascii_val;
}