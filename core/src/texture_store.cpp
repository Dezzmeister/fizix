#include "texture_store.h"

texture_store::texture_store(event_buses &_buses) :
	event_listener<program_start_event>(&_buses.lifecycle, -100),
	event_listener<program_stop_event>(&_buses.lifecycle)
{
	event_listener<program_start_event>::subscribe();
	event_listener<program_stop_event>::subscribe();
}

int texture_store::handle(program_start_event &event) {
	textures["flat_normal"] = std::make_unique<texture>("assets/textures/flat_normal.png");
	textures["flat_specular_0.2"] = std::make_unique<texture>("assets/textures/flat_specular_0.2.png");
	textures["wall"] = std::make_unique<texture>("assets/textures/wall.jpg");
	textures["container2"] = std::make_unique<texture>("assets/textures/container2.png");
	textures["container2_specular"] = std::make_unique<texture>("assets/textures/container2_specular.png");
	textures["brickwall"] = std::make_unique<texture>("assets/textures/brickwall.jpg");
	textures["brickwall_normal"] = std::make_unique<texture>("assets/textures/brickwall_normal.jpg");

	event.textures = this;

	return 0;
}

const texture& texture_store::store(const std::string &name, texture tex) {
	textures[name] = std::make_unique<texture>(std::move(tex));

	return *textures.at(name);
}

const texture& texture_store::get(const std::string &name) const {
	return *textures.at(name);
}

int texture_store::handle(program_stop_event&) {
	textures.clear();

	return 0;
}
