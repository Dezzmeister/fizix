#pragma once
#include <bitset>
#include "gl.h"
#include "events.h"

class key_controller :
	public event_listener<pre_render_pass_event>,
	public event_listener<mouse_lock_event>,
	public event_listener<mouse_unlock_event>
{
public:
	key_controller(event_buses &_buses, std::vector<short> _watched_keys);

	int handle(pre_render_pass_event &event) override;
	int handle(mouse_lock_event &event) override;
	int handle(mouse_unlock_event &event) override;

private:
	event_buses &buses;
	std::bitset<KEY_MAX> keys{};
	std::vector<short> watched_keys;
	bool is_mouse_locked{};
};

class screen_controller :
	public event_listener<pre_render_pass_event>,
	public event_listener<post_processing_event>,
	public event_listener<program_start_event>
{
public:
	screen_controller(event_buses &_buses);

	int handle(pre_render_pass_event &event) override;
	int handle(post_processing_event &event) override;
	int handle(program_start_event &event) override;

private:
	event_buses &buses;
	int screen_width;
	int screen_height;
};

class mouse_controller :
	public event_listener<pre_render_pass_event>,
	public event_listener<keydown_event>
{
public:
	mouse_controller(
		event_buses &_buses,
		std::vector<uint8_t> _watched_buttons,
		// -1 to disable mouse lock
		short _mouse_unlock_key
	);

	int handle(pre_render_pass_event &event) override;
	int handle(keydown_event &event) override;

	void set_scroll_offset(const float x, const float y);

private:
	event_buses &buses;
	std::bitset<KEY_MAX> buttons{};
	std::vector<uint8_t> watched_buttons;
	glm::vec2 scroll_offset{};
	platform::window * mouse_locked_window{};
	short mouse_unlock_key;
};

