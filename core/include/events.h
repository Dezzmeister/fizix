// This file defines several events that are used throughout the program. Events
// are logically grouped into "event buses" based on their category. For example:
// a "render" event bus handles any events that have to do with drawing to the screen,
// an "input" event bus handles any events that have to do with user input, etc.
// Events can be anything, but so far it's most convenient for them to be small data
// classes. Some events can have "effects" that allow them to do something before and/or
// after they are fired. For example, the event that fires before every render pass has
// an effect that it uses to time the previous render pass. These "effectful" events are
// most useful if they're created once, outside of the scope from which they are fired.
//
// A note about input events: although an input can be made at any time, input events are
// only fired by "controllers" (see controllers.h) at the beginning of a frame. Events are
// synchronous, so input event handlers will only be called at the beginning of a frame,
// when the input event(s) is/are fired.
#pragma once
#include <chrono>
#include "gl.h"
#include <GLFW/glfw3.h>
#include "event.h"
#include "shader_program.h"

// The various "store" classes own common resources of a given type and
// control their lifetimes. These classes load some predefined resources
// on `program_start_event`, and they free these resources on
// `program_stop_event`. These resources are passed to objects on `draw_event`
// and other render events.
class shader_store;
class texture_store;

class renderer2d;

class light;

class hardware_constants;

// Fired before the game loop starts, but after the GL and GDI contexts are
// created. If the user has shaders and textures that they need to load, they
// can do that on this event.
struct program_start_event {
	GLFWwindow * window;
	shader_store * shaders{ nullptr };
	texture_store * textures{ nullptr };
	hardware_constants * hardware_consts{ nullptr };
	renderer2d * draw2d;
	int screen_width;
	int screen_height;
};

// Fired after the game loop ends, but before the GL context is destroyed.
struct program_stop_event {
	int reason;

	program_stop_event(int _reason) : reason(_reason) {}
};

// Fired at the beginning of every render pass. This event times the previous
// render pass (see `delta`).
struct pre_render_pass_event {
	GLFWwindow * window;
	hardware_constants * hardware_consts;
	// The time in ms since the beginning of the previous render pass (i.e., since
	// the last moment this event was fired).
	std::chrono::milliseconds delta;
	int screen_width;
	int screen_height;

	pre_render_pass_event(GLFWwindow * _window, hardware_constants * _hardware_consts) :
		window(_window),
		hardware_consts(_hardware_consts),
		delta(),
		screen_width(0),
		screen_height(0),
		prev_render_pass(std::chrono::steady_clock::now())
	{}

	int before_fire() {
		std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
		delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - prev_render_pass);
		prev_render_pass = now;

		return 0;
	}

	int after_fire() {
		return 0;
	}

private:
	std::chrono::time_point<std::chrono::steady_clock> prev_render_pass;
};
static_assert(effectful<pre_render_pass_event>);

// Fired after `pre_render_pass_event` and before `post_processing_event`. Anything that
// needs to render to the 3D scene should use this event as a call to do so.
struct draw_event {
	GLFWwindow * window;
	shader_store &shaders;
	texture_store &textures;
	// The view matrix that the camera is using for this render pass. The camera should
	// set this before any other listeners handle this event.
	glm::mat4 * view;
	glm::mat4 * inv_view;

	draw_event(GLFWwindow * _window, shader_store &_shaders, texture_store &_textures) :
		window(_window),
		shaders(_shaders),
		textures(_textures),
		view(nullptr),
		inv_view(nullptr)
	{}
};

// Fired after `draw_event`. Unlike `draw_event`, this should be used as a call to do
// any post-processing, which may or may not have anything to do with the 3D scene.
// This is the last event that is fired before the window buffers are swapped and the
// new front buffer is drawn to the screen.
struct post_processing_event {
	GLFWwindow * window;
	shader_store &shaders;
	texture_store &textures;
	renderer2d &draw2d;
	int screen_width;
	int screen_height;

	post_processing_event(
		GLFWwindow * _window,
		shader_store &_shaders,
		texture_store &_textures,
		renderer2d &_draw2d
	) :
		window(_window),
		shaders(_shaders),
		textures(_textures),
		draw2d(_draw2d),
		screen_width(0),
		screen_height(0)
	{}
};

// Fired after the buffers are swapped, at the end of the render pass. This is the last event
// fired during a render pass. At the moment this is unused.
struct post_render_pass_event {
};

// Fired whenever the size of the window changes. The dimensions given are those of the
// drawable area, in pixels.
struct screen_resize_event {
	const int old_width;
	const int old_height;
	const int new_width;
	const int new_height;

	screen_resize_event(
		int _old_width,
		int _old_height,
		int _new_width,
		int _new_height
	) : old_width(_old_width),
		old_height(_old_height),
		new_width(_new_width),
		new_height(_new_height)
	{}
};

// Fired whenever a shader is used. 
// TODO: Come up with a way for the camera to skip setting uniforms when the shader
// does not use them
struct shader_use_event {
	const shader_program &shader;

	shader_use_event(const shader_program &_shader) : shader(_shader) {}
};

// Fired whenever a key is pressed. The event is not fired again until the next time
// that the key is pressed.
struct keydown_event {
	const short key;
	const bool is_mouse_locked;

	keydown_event(
		const short _key,
		bool _is_mouse_locked
	) :
		key(_key),
		is_mouse_locked(_is_mouse_locked)
	{}
};

// Fired whenever a key is released.
struct keyup_event {
	const short key;
	const bool is_mouse_locked;

	keyup_event(
		const short _key,
		bool _is_mouse_locked
	) :
		key(_key),
		is_mouse_locked(_is_mouse_locked)
	{}
};

// Fired whenever a mouse button is pressed.
struct mousedown_event {
	const uint8_t button;
	const bool is_mouse_locked;

	mousedown_event(
		const uint8_t _button,
		bool _is_mouse_locked
	) :
		button(_button),
		is_mouse_locked(_is_mouse_locked)
	{}
};

// Fired whenever a mouse button is released.
struct mouseup_event {
	const uint8_t button;
	const bool is_mouse_locked;

	mouseup_event(
		const uint8_t _button,
		bool _is_mouse_locked
	) :
		button(_button),
		is_mouse_locked(_is_mouse_locked)
	{}
};

// Fired whenever the mouse is scrolled.
struct mouse_scroll_event {
	const glm::vec2 offset;
	const bool is_mouse_locked;

	mouse_scroll_event(
		const glm::vec2 &_offset,
		bool _is_mouse_locked
	) :
		offset(_offset),
		is_mouse_locked(_is_mouse_locked)
	{}
};

// Fired when the cursor is locked.
struct mouse_lock_event {
	GLFWwindow * window;

	mouse_lock_event(GLFWwindow * _window) : window(_window) {}
};

// Fired when the cursor is unlocked.
struct mouse_unlock_event {
	GLFWwindow * window;

	mouse_unlock_event(GLFWwindow * _window) : window(_window) {}
};

class player;

// Fired whenever a player looks in a different direction.
struct player_look_event {
	player &p;
	glm::vec3 pos;
	glm::vec3 dir;
	glm::vec3 old_dir;
	const glm::mat4 &view;

	player_look_event(player &_p, const glm::vec3 &_pos, const glm::vec3 &_dir, const glm::vec3 &_old_dir, const glm::mat4 &_view) :
		p(_p),
		pos(_pos),
		dir(_dir),
		old_dir(_old_dir),
		view(_view)
	{}
};

// Fired whenever a player moves.
struct player_move_event {
	player &p;
	glm::vec3 pos;
	glm::vec3 old_pos;
	glm::vec3 dir;

	player_move_event(player &_p, const glm::vec3 &_pos, const glm::vec3 &_old_pos, const glm::vec3 &_dir) :
		p(_p),
		pos(_pos),
		old_pos(_old_pos),
		dir(_dir)
	{}
};

// Fired when the player spawns.
struct player_spawn_event {
	player &p;
	glm::vec3 pos;
	glm::vec3 dir;

	player_spawn_event(player &_p, const glm::vec3 &_pos, const glm::vec3 &_dir) :
		p(_p),
		pos(_pos),
		dir(_dir)
	{}
};

using lifecycle_event_bus = event_bus<program_start_event, program_stop_event>;
using render_event_bus = event_bus<
	pre_render_pass_event,
	draw_event,
	post_processing_event,
	post_render_pass_event,
	shader_use_event,
	screen_resize_event
>;
using input_event_bus = event_bus<
	keydown_event,
	keyup_event,
	mousedown_event,
	mouseup_event,
	mouse_scroll_event,
	mouse_lock_event,
	mouse_unlock_event
>;
using player_event_bus = event_bus<
	player_look_event,
	player_move_event,
	player_spawn_event
>;

struct event_buses {
	lifecycle_event_bus lifecycle;
	render_event_bus render;
	input_event_bus input;
	player_event_bus player;
};
