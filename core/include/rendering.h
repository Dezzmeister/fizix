#pragma once

class render_pass_state {
private:
	unsigned int used_texture_units{ 0 };

public:
	const int default_sampler2d_tex_unit;
	const int default_cubesampler_tex_unit;
	const int max_tex_units;

	render_pass_state(
		int _default_sampler2d_tex_unit,
		int _default_cubesampler_tex_unit,
		int _max_tex_units
	);

	unsigned int next_texture_unit();
	void reset();
};
