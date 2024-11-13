#pragma once
#include "events.h"
#include "texture.h"
#include "world.h"
#include "custom_events.h"

class tool {
public:
	const texture &icon;
	const std::string name;
	const std::string tooltip;

	tool(
		const texture &_icon,
		const std::string &_name,
		const std::string &_tooltip
	);
	virtual ~tool() = default;

	virtual void activate() = 0;
	virtual void deactivate() = 0;
	virtual bool is_active() const = 0;
};

class toolbox :
	public event_listener<program_start_event>,
	public event_listener<mouse_scroll_event>
{
public:
	toolbox(
		event_buses &_buses,
		custom_event_bus &_custom_bus,
		world &_mesh_world
	);

	int handle(program_start_event &event) override;
	int handle(mouse_scroll_event &event) override;

private:
	event_buses &buses;
	custom_event_bus &custom_bus;
	world &mesh_world;
	std::vector<std::unique_ptr<tool>> tools{};
	size_t curr_tool{};

	void register_tool(const tool * t);
};
