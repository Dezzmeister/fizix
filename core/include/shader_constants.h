#pragma once
#include "util.h"

namespace constants {
	// I'm using uniform locations in a compile-time map here after doing some profiling:
	// I found that the `unordered_map` lookup to get the location of a uniform given its name
	// was costing some significant time each frame.
	constexpr util::str_kv_pair<int> shader_locs[] = {
		{ "model", 20 },
		{ "view", 21 },
		{ "projection", 22 },
		{ "normal_mat", 23 },
		{ "inv_view", 29 },
		{ "num_lights", 149 },
		{ "lights", 150 },
		{ "alpha", 19 },

		{ "_color_mat.ambient", 24 },
		{ "_color_mat.diffuse", 25 },
		{ "_color_mat.specular", 26 },
		{ "_color_mat.shininess", 27 },

		{ "_texture_mat.diffuse", 24 },
		{ "_texture_mat.specular", 25 },
		{ "_texture_mat.normal", 26 },
		{ "_texture_mat.shininess", 27 },

		{ "light.type", 0 },
		{ "light.pos", 1 },
		{ "light.dir", 2 },
		{ "light.ambient", 3 },
		{ "light.diffuse", 4 },
		{ "light.specular", 5 },
		{ "light.att_c", 6 },
		{ "light.att_l", 7 },
		{ "light.att_q", 8 },
		{ "light.inner_cutoff", 9 },
		{ "light.outer_cutoff", 10 },
		// By `sizeof` I just mean the number of uniform locations that a struct
		// occupies, not its physical size in memory
		{ "sizeof(light)", 11 },

		{ "view_proj", 24 },
		{ "light_pos", 8 },
		{ "far_plane", 9 },

		{ "shadow_casters", 32 },
		{ "shadow_caster.light_space", 0 },
		{ "shadow_caster.depth_map", 1 },
		{ "shadow_caster.enabled", 2 },
		{ "shadow_caster.cube_depth_map", 3 },
		{ "shadow_caster.far_plane", 4 },
		{ "sizeof(shadow_caster)", 5 },

		{ "tex_sampler_tex", 8 },
		{ "cube_sampler_cubemap", 8 },
		{ "cube_sampler_u_filter", 9 },
		{ "cube_sampler_v_filter", 10 },

		{ "max_particle_size", 1 }
	};
};
