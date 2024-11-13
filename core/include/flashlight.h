#pragma once
#include <memory>
#include "events.h"
#include "player.h"
#include "spotlight.h"
#include "world.h"

class flashlight : public event_listener<pre_render_pass_event>, public event_listener<keydown_event> {
public:
	std::unique_ptr<spotlight> light;

	flashlight(event_buses &_buses, const player &_pl, world &_w, const short _toggle_key);

	int handle(pre_render_pass_event &event) override;

	int handle(keydown_event &event) override;

private:
	const player &pl;
	world &w;
	const short toggle_key;
	bool enabled;
	bool added_to_world;
};
