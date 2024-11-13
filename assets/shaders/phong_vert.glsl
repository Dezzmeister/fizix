// TODO: Profile and optimize the Phong shaders
#extension GL_ARB_explicit_uniform_location : enable

// TODO: Share code
struct shadow_caster {
	mat4 light_space;
	sampler2D depth_map;
	bool enabled;
	// The cubemap and far plane are undefined for anything other than a point light
	samplerCube cube_depth_map;
	float far_plane;
};

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 tex_coords_in;
#ifdef USE_MAPS
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;
#endif

#ifdef INSTANCED
layout(location = 3) in vec4 model_0;
layout(location = 4) in vec4 model_1;
layout(location = 5) in vec4 model_2;
layout(location = 6) in vec4 model_3;
layout(location = 7) in vec4 inv_model_0;
layout(location = 8) in vec4 inv_model_1;
layout(location = 9) in vec4 inv_model_2;
layout(location = 10) in vec4 inv_model_3;
#else
layout(location = 20) uniform mat4 model;
layout(location = 23) uniform mat3 normal_mat;
#endif

layout(location = 21) uniform mat4 view;
layout(location = 22) uniform mat4 projection;

layout(location = 29) uniform mat4 inv_view;

layout(location = 31) uniform int num_shadow_casters;
layout(location = 32) uniform shadow_caster shadow_casters[MAX_LIGHTS];

out vec3 frag_pos_world;
// In view space
out vec3 frag_pos_view;
out vec3 frag_normal;
out vec4 frag_pos_light_space[MAX_LIGHTS];

#ifdef USE_MAPS
out vec2 tex_coords;
out mat3 tbn;
#endif

void main() {
#ifdef INSTANCED
	mat4 model = mat4(model_0, model_1, model_2, model_3);
	mat4 inv_model = mat4(inv_model_0, inv_model_1, inv_model_2, inv_model_3);
	mat3 normal_mat = mat3(transpose(inv_model * inv_view));
#endif
	vec4 world_pos = model * vec4(pos, 1.0);
	vec4 view_pos = view * world_pos;

	gl_Position = projection * view_pos;
	frag_pos_world = vec3(world_pos);
	frag_pos_view = vec3(view_pos);
	frag_normal = normal_mat * normal;

	for (int i = 0; i < MAX_LIGHTS; i++) {
		if (shadow_casters[i].enabled) {
			frag_pos_light_space[i] = shadow_casters[i].light_space * world_pos;
		}
	}

#ifdef USE_MAPS
	tex_coords = tex_coords_in;
	vec3 t = normalize(vec3(view * model * vec4(tangent, 0.0)));
	vec3 b = normalize(vec3(view * model * vec4(bitangent, 0.0)));
	vec3 n = normalize(vec3(view * model * vec4(normal, 0.0)));

	tbn = mat3(t, b, n);
#endif
}
