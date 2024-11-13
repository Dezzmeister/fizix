#extension GL_ARB_explicit_uniform_location : enable

struct material {
#ifdef USE_MAPS
	sampler2D diffuse;
	sampler2D specular;
	sampler2D normal;
#else
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
#endif
	float shininess;
};

const int point_light_type = 0;
const int dir_light_type = 1;
const int spotlight_type = 2;

struct light {
	// offset = 0
	int type;

	// offset = 1
	vec3 pos;
	// offset = 2
	vec3 dir;

	// offset = 3
	vec3 ambient;
	// offset = 4
	vec3 diffuse;
	// offset = 5
	vec3 specular;

	// Attenuation factors
	// offset = 6
	float att_c;
	// offset = 7
	float att_l;
	// offset = 8
	float att_q;

	// Cosine of spotlight cutoff angles
	// offset = 9
	float inner_cutoff;
	// offset = 10
	float outer_cutoff;
};

// TODO: Share code
struct shadow_caster {
	mat4 light_space;
	sampler2D depth_map;
	bool enabled;
	// The cubemap and far plane are undefined for anything other than a point light
	samplerCube cube_depth_map;
	float far_plane;
};

in vec3 frag_pos_world;
in vec3 frag_pos_view;
in vec3 frag_normal;
in vec4 frag_pos_light_space[MAX_LIGHTS];

#ifdef USE_MAPS
in vec2 tex_coords;
in mat3 tbn;
#endif

out vec4 frag_color;

layout(location = 21) uniform mat4 view;
layout(location = 24) uniform material mat;

layout(location = 149) uniform int num_lights;
layout(location = 150) uniform light lights[MAX_LIGHTS];

layout(location = 32) uniform shadow_caster shadow_casters[MAX_LIGHTS];

#ifdef TRANSPARENCY
layout(location = 19) uniform float alpha;
#endif

float compute_point_shadow(int i, vec3 light_dir, vec3 norm) {
	vec3 frag_to_light = frag_pos_world - lights[i].pos;
	float current_depth = length(frag_to_light);
	float norm_depth = texture(shadow_casters[i].cube_depth_map, frag_to_light).r;

	if (norm_depth == 1.0) {
		return 0.0;
	}

	float shadow = 0.0;
	float samples = 4.0;
	// TODO: Make this customizable
	float offset = 0.01;
	float bias = max(0.19 * (1.0 - dot(norm, light_dir)), 0.05);

	for (float x = -offset; x < offset; x += offset / (samples * 0.5)) {
		for (float y = -offset; y < offset; y += offset / (samples * 0.5)) {
			for (float z = -offset; z < offset; z += offset / (samples * 0.5)) {
				float closest_depth = texture(shadow_casters[i].cube_depth_map, frag_to_light + vec3(x, y, z)).r * shadow_casters[i].far_plane;

				if (current_depth - bias > closest_depth) {
					shadow += 1.0;
				}
			}
		}
	}

	return shadow / (samples * samples * samples);
}

float compute_shadow(int i, vec3 light_dir, vec3 norm) {
	if (! shadow_casters[i].enabled) {
		return 0.0;
	}

	if (lights[i].type == point_light_type) {
		return compute_point_shadow(i, light_dir, norm);
	}

	if (frag_pos_light_space[i].z > 1.0) {
		return 0.0;
	}

	vec3 pov_light_pos = frag_pos_light_space[i].xyz / frag_pos_light_space[i].w;
	pov_light_pos = pov_light_pos * 0.5 + 0.5;

	float current_depth = pov_light_pos.z;

	// I don't like this; it's a hacky way to avoid shadow acne and it's easily broken
	float bias = max(0.0005 * (1.0 - dot(norm, light_dir)), 0.00005);

	float shadow = 0.0;
	vec2 texel_size = 1.0 / textureSize(shadow_casters[i].depth_map, 0);

	// TODO: Use shadow samplers
	for (float x = -1.5; x <= 1.5; x += 1.0) {
		for (float y = -1.5; y <= 1.5; y += 1.0) {
			float pcf_depth = texture(shadow_casters[i].depth_map, pov_light_pos.xy + vec2(x, y) * texel_size).r;
			shadow += current_depth - bias > pcf_depth ? 1.0 : 0.0;
		}
	}

	return shadow / 16.0;
}

// Lighting computations are done in view space so that we don't need to know
// the camera pos
void main() {
	vec3 color_out = vec3(0.0, 0.0, 0.0);
	vec3 view_pos = vec3(0.0, 0.0, 0.0);
	vec3 norm = normalize(frag_normal);

#ifdef USE_MAPS
	norm = texture(mat.normal, tex_coords).rgb;
	norm = norm * 2.0 - 1.0;
	norm = normalize(tbn * norm);

	vec3 ambient_color = vec3(texture(mat.diffuse, tex_coords));
	vec3 diffuse_color = ambient_color;
	vec3 specular_color = vec3(texture(mat.specular, tex_coords));
#else
	vec3 ambient_color = mat.ambient;
	vec3 diffuse_color = mat.diffuse;
	vec3 specular_color = mat.specular;
#endif

	for (int i = 0; i < num_lights; i++) {
		light l = lights[i];
		vec3 light_dir;

		if (l.type == point_light_type || l.type == spotlight_type) {
			vec3 view_light_pos = vec3(view * vec4(l.pos, 1.0));

			light_dir = normalize(view_light_pos - frag_pos_view);
		} else if (l.type == dir_light_type) {
			vec3 view_light_dir = vec3(view * vec4(l.dir, 0.0));

			light_dir = normalize(-view_light_dir);
		}

		vec3 view_dir = normalize(view_pos - frag_pos_view);
		vec3 halfway_dir = normalize(light_dir + view_dir);
		vec3 reflect_dir = reflect(-light_dir, norm);

		float diff = max(dot(norm, light_dir), 0.0);

		// This check is not part of the Blinn-Phong lighting model. I added this to prevent
		// drawing specular highlights when the light and the viewer are on opposite sides
		// of the fragment; otherwise, the viewer may see specular highlights on a surface that
		// obscures the light. This is especially noticeable with the sharper Blinn-Phong 
		// highlights (as opposed to Phong highlights).
		float light_side_test = dot(light_dir, norm);
		float view_side_test = dot(view_dir, norm);
		float spec = 0.0;

		if ((light_side_test > 0) == (view_side_test > 0)) {
			spec = pow(max(dot(norm, halfway_dir), 0.0), mat.shininess);
		}

		vec3 ambient = l.ambient * ambient_color;
		vec3 diffuse = l.diffuse * diff * diffuse_color;
		vec3 specular = l.specular * spec * specular_color;

		if (l.type == spotlight_type) {
			vec3 view_spotlight_dir = vec3(view * vec4(l.dir, 0.0));
			float theta = dot(light_dir, normalize(-view_spotlight_dir));
			float epsilon = l.inner_cutoff - l.outer_cutoff;
			float intensity = clamp((theta - l.outer_cutoff) / epsilon, 0.0, 1.0);

			diffuse *= intensity;
			specular *= intensity;
		}

		if (l.type == point_light_type || l.type == spotlight_type) {
			float d = length(l.pos - frag_pos_world);
			float attenuation = 1.0 / (l.att_c + (l.att_l * d) + (l.att_q * d * d));

			ambient *= attenuation;
			diffuse *= attenuation;
			specular *= attenuation;
		}

		float shadow = compute_shadow(i, light_dir, norm);

		color_out += ambient + (1.0 - shadow) * (diffuse + specular);
	}

#ifndef TRANSPARENCY
	float alpha = 1.0;
#endif

	frag_color = vec4(color_out, alpha);
}