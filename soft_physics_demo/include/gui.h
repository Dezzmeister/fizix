#pragma once
#include <chrono>
#include "events.h"
#include "physics/particle.h"
#include "draw2d.h"
#include "custom_events.h"

class gui :
	public event_listener<program_start_event>,
	public event_listener<pre_render_pass_event>,
	public event_listener<post_processing_event>,
	public event_listener<tool_register_event>
{
public:
	gui(event_buses &_buses, custom_event_bus &_custom_bus);

	int handle(program_start_event &event) override;
	int handle(pre_render_pass_event &event) override;
	int handle(post_processing_event &event) override;
	int handle(tool_register_event &event) override;

private:
	event_buses &buses;
	const font * debug_font{ nullptr };
	std::chrono::time_point<std::chrono::steady_clock> last_fps_update{};
	uint32_t frames{ 0 };
	std::vector<const tool *> tools{};
	const texture * selected_tool_bg{ nullptr };
	const texture * unselected_tool_bg{ nullptr };
	float fps{ 0.0f };

	void draw_fps_count(const post_processing_event &event) const;
	void draw_crosshair(const post_processing_event &event) const;
	void draw_tools(const post_processing_event &event) const;

	void draw_tool_info(const post_processing_event &event, const tool * t) const;
};
