#pragma once
#include <string>
#include <unordered_map>
#include "events.h"
#include "shader_program.h"

class shader_store : public event_listener<program_start_event>, public event_listener<program_stop_event> {
public:
	std::unordered_map<std::string, shader_program> shaders;

	shader_store(event_buses &_buses);

	// Shaders are loaded on program start
	int handle(program_start_event &event) override;

	// Shaders are unloaded on program stop
	int handle(program_stop_event &event) override;
};

