#pragma once
#include <bitset>
#include "gl.h"
#include <GLFW/glfw3.h>
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
	std::bitset<GLFW_KEY_LAST> keys{};
	std::vector<short> watched_keys;
	bool is_mouse_locked{ false };
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
	public event_listener<program_start_event>,
	public event_listener<keydown_event>
{
public:
	mouse_controller(
		event_buses &_buses,
		std::vector<uint8_t>
		_watched_buttons,
		short _mouse_unlock_key
	);
	~mouse_controller();

	// These are deleted so that mouse controllers can receive scroll input,
	// which is only provided by GLFW via a callback. Each mouse_controller
	// adds itself to a list, so moving or copying a mouse_controller would
	// require the list to be updated. It's not important enough to implement
	// now.
	mouse_controller(const mouse_controller &other) = delete;
	mouse_controller(mouse_controller &&other) = delete;
	mouse_controller& operator=(const mouse_controller &other) = delete;
	mouse_controller& operator=(mouse_controller &&other) = delete;

	int handle(pre_render_pass_event &event) override;
	int handle(program_start_event &event) override;
	int handle(keydown_event &event) override;

	void set_scroll_offset(const float x, const float y);

private:
	event_buses &buses;
	std::bitset<GLFW_MOUSE_BUTTON_LAST> buttons{};
	std::vector<uint8_t> watched_buttons;
	glm::vec2 scroll_offset{};
	GLFWwindow * mouse_locked_window{ nullptr };
	short mouse_unlock_key;
	bool is_mouse_locked{ false };
};

