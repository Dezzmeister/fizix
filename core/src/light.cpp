#include "light.h"

light_properties::light_properties(glm::vec3 _ambient, glm::vec3 _diffuse, glm::vec3 _specular) :
	ambient(_ambient),
	diffuse(_diffuse),
	specular(_specular)
{}

attenuation_factors::attenuation_factors(float _constant, float _linear, float _quadratic) :
	constant(_constant),
	linear(_linear),
	quadratic(_quadratic)
{}

light::light(light_type _type) :
	type(_type),
	shadow_fbo(0, [](unsigned int _fbo) {
		glDeleteFramebuffers(1, &_fbo);
	})
{}

bool light::casts_shadow() const {
	return shadow_fbo != 0;
}

unsigned int light::get_shadow_fbo() const {
	return shadow_fbo;
}

bool operator==(const light_properties &a, const light_properties &b) {
	return (a.ambient == b.ambient) && (a.diffuse == b.diffuse) && (a.specular == b.specular);
}

bool operator==(const attenuation_factors &a, const attenuation_factors &b) {
	return (a.constant == b.constant) && (a.linear == b.linear) && (a.quadratic == b.quadratic);
}

bool operator==(const light &a, const light &b) {
	return a.is_eq(b);
}
