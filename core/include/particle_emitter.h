#pragma once
#include "events.h"
#include "shader_program.h"

class particle_emitter {
public:
	particle_emitter() = default;
	virtual ~particle_emitter() = default;

	virtual void start() = 0;
	virtual void stop() = 0;
	virtual bool is_done() const = 0;

	virtual void update(float millis) = 0;
	virtual void prepare_draw(draw_event &event, const shader_program &shader) const = 0;
	virtual void draw() const = 0;

	virtual const std::string& shader_name() const = 0;
};
