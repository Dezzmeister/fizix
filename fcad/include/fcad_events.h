#pragma once
#include <event.h>
#include <traits.h>
#include <world.h>
#include <physics/math.h>
#include <physics/collision/vclip.h>

using namespace phys;
using namespace vclip;

enum class edit_mode {
	Normal,
	Command,
	Select
};

class platform_bridge;
class geometry_controller;
class edit_history_controller;
class file_controller;
class mode_controller;
class camera_controller;
class action_controller;
class command_controller;

struct fcad_start_event {
	platform_bridge &platform;
	geometry_controller &gc;
	edit_history_controller &edit_history;
	file_controller &fc;
	mode_controller &mode;
	camera_controller &camera;
	action_controller &actions;
	command_controller &commands;
	world &mesh_world;

	fcad_start_event(
		platform_bridge &_platform,
		geometry_controller &_gc,
		edit_history_controller &_edit_history,
		file_controller &_fc,
		mode_controller &_mode,
		camera_controller &_camera,
		action_controller &_actions,
		command_controller &_commands,
		world &_mesh_world
	) :
		platform(_platform),
		gc(_gc),
		edit_history(_edit_history),
		fc(_fc),
		mode(_mode),
		camera(_camera),
		actions(_actions),
		commands(_commands),
		mesh_world(_mesh_world)
	{}
};

struct window_input_event {
	const char c;

	window_input_event(char _c) : c(_c) {}
};

struct mode_switch_event {
	const edit_mode old_mode;
	const edit_mode new_mode;
	const std::wstring hint;

	mode_switch_event(
		edit_mode _old_mode,
		edit_mode _new_mode,
		const std::wstring &_hint = L""
	) :
		old_mode(_old_mode),
		new_mode(_new_mode),
		hint(_hint)
	{}
};

struct command_cancel_event {
	// This will most likely not be a complete command
	const std::wstring command_buf;

	command_cancel_event(const std::wstring &_command_buf) :
		command_buf(_command_buf) {}
};

struct command_input_event {
	const std::wstring command_buf;

	command_input_event(const std::wstring &_command_buf) :
		command_buf(_command_buf) {}
};

struct command_submit_event {
	const std::wstring command;

	command_submit_event(const std::wstring &_command) :
		command(_command) {}
};

struct new_vertex_event {
	const vec3 vertex;

	new_vertex_event(const vec3 &_vertex) :
		vertex(_vertex) {}
};

struct new_edge_event {
	const edge e;

	new_edge_event(const edge &_e) :
		e(_e) {}
};

struct new_face_event {
	const face f;

	new_face_event(const face &_f) :
		f(_f) {}
};

struct delete_vertex_event {
	const size_t vertex_idx;

	delete_vertex_event(size_t _vertex_idx) :
		vertex_idx(_vertex_idx) {}
};

struct delete_edge_event {
	const edge e;

	delete_edge_event(const edge &_e) :
		e(_e) {}
};

struct delete_face_event {
	const face f;

	delete_face_event(const face &_f) :
		f(_f) {}
};

struct camera_move_event {
	const vec3 &pos;
	const vec3 &up;
	const vec3 &right;
	const vec3 &target;
	const mat4 &view;
	const mat4 &inv_view;
	const mat4 &proj;

	camera_move_event(
		const vec3 &_pos,
		const vec3 &_up,
		const vec3 &_right,
		const vec3 &_target,
		const mat4 &_view,
		const mat4 &_inv_view,
		const mat4 &_proj
	) :
		pos(_pos),
		up(_up),
		right(_right),
		target(_target),
		view(_view),
		inv_view(_inv_view),
		proj(_proj)
	{}
};

// Fired before a replay file is written. If this event is cancelled, the replay
// file won't be written.
struct write_replay_file_event {
	const std::wstring path;

	write_replay_file_event(const std::wstring &_path) :
		path(_path) {}
};

// Fired before a replay file is read and played back. If this event is cancelled,
// the replay file won't be read.
struct read_replay_file_event {
	const std::wstring path;

	read_replay_file_event(const std::wstring &_path) :
		path(_path) {}
};

using fcad_event_bus = event_bus<
	fcad_start_event,
	window_input_event,
	mode_switch_event,
	command_cancel_event,
	command_input_event,
	command_submit_event,
	new_vertex_event,
	new_edge_event,
	new_face_event,
	delete_vertex_event,
	delete_edge_event,
	delete_face_event,
	camera_move_event,
	write_replay_file_event,
	read_replay_file_event
>;

namespace traits {
	template <>
	std::string to_string(const edit_mode &mode, size_t indent);
}
