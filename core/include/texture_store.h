#pragma once
#include <string>
#include <unordered_map>
#include "events.h"
#include "texture.h"

class texture_store : public event_listener<program_start_event>, public event_listener<program_stop_event> {
public:
	texture_store(event_buses &_buses);

	const texture& store(const std::string &name, texture tex);
	const texture& get(const std::string &name) const;

	int handle(program_start_event &event) override;
	int handle(program_stop_event &event) override;

private:
	std::unordered_map<std::string, std::unique_ptr<texture>> textures{};
};

