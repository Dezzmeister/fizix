#extension GL_ARB_explicit_uniform_location : enable

layout(location = 0) in vec3 pos;

#ifdef INSTANCED
layout(location = 3) in vec4 model_0;
layout(location = 4) in vec4 model_1;
layout(location = 5) in vec4 model_2;
layout(location = 6) in vec4 model_3;
#else
layout(location = 20) uniform mat4 model;
#endif

void main() {
#ifdef INSTANCED
	mat4 model = mat4(model_0, model_1, model_2, model_3);
#endif

	gl_Position = model * vec4(pos, 1.0);
}
