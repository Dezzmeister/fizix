#include "rendering.h"

render_pass_state::render_pass_state(
	int _default_sampler2d_tex_unit,
	int _default_cubesampler_tex_unit,
	int _max_tex_units
) :
	default_sampler2d_tex_unit(_default_sampler2d_tex_unit),
	default_cubesampler_tex_unit(_default_cubesampler_tex_unit),
	max_tex_units(_max_tex_units)
{}

unsigned int render_pass_state::next_texture_unit() {
	unsigned int out = used_texture_units;

	// TODO: Max texture units > 16
	used_texture_units = (used_texture_units + 1) & 0xF;

	return out;
}

void render_pass_state::reset() {
	used_texture_units = 0;
}