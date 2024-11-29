#include "gl.h"
#include "hardware_constants.h"
#include "logging.h"

hardware_constants::hardware_constants(event_buses &_buses) :
	event_listener<program_start_event>(&_buses.lifecycle, -100)
{
	event_listener<program_start_event>::subscribe();
}

int hardware_constants::handle(program_start_event &event) {
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_texture_units);

	initialized = true;
	event.hardware_consts = this;

	const GLubyte * vendor = glGetString(GL_VENDOR);
	const GLubyte * renderer = glGetString(GL_RENDERER);
	const GLubyte * version = glGetString(GL_VERSION);
	const GLubyte * shading_lang_version = glGetString(GL_SHADING_LANGUAGE_VERSION);
	int max_vert_uniforms, max_frag_uniforms, max_geom_uniforms;
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &max_vert_uniforms);
	glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &max_frag_uniforms);
	glGetIntegerv(GL_MAX_GEOMETRY_UNIFORM_COMPONENTS, &max_geom_uniforms);

	logger::info("Graphics vendor: " + traits::to_string(vendor));
	logger::info("Graphics device: " + traits::to_string(renderer));
	logger::info("OpenGL version: " + traits::to_string(version));
	logger::info("GLSL version: " + traits::to_string(shading_lang_version));
	logger::info("Max texture units: " + traits::to_string(max_texture_units));
	logger::info("Max vertex uniform components: " + traits::to_string(max_vert_uniforms));
	logger::info("Max fragment uniform components: " + traits::to_string(max_frag_uniforms));
	logger::info("Max geometry uniform components: " + traits::to_string(max_geom_uniforms));

	return 0;
}

int hardware_constants::get_max_texture_units() const {
	guard();

	return max_texture_units;
}

void hardware_constants::guard() const {
	if (! initialized) {
		// TODO: Errors
		throw "Hardware constants are not initialized";
	}
}

