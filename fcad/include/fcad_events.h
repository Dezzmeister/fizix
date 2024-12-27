#pragma once
#include <event.h>
#include <traits.h>
#include <physics/math.h>
#include <physics/collision/vclip.h>

using namespace phys;
using namespace vclip;

enum class edit_mode {
	Normal,
	Command,
	Select
};

class geometry_controller;

struct fcad_start_event {
	geometry_controller &gc;

	fcad_start_event(geometry_controller &_gc) : gc(_gc) {}
};

struct window_input_event {
	const char c;

	window_input_event(char _c) : c(_c) {}
};

// Initiates a mode switch
struct set_mode_event {
	const edit_mode new_mode;
	const std::wstring hint;

	set_mode_event(
		edit_mode _new_mode,
		const std::wstring &_hint = L""
	) :
		new_mode(_new_mode),
		hint(_hint)
	{}
};

// TODO: Delete this and use only set_mode_event?
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

struct set_camera_target_event {
	const vec3 &new_target;

	set_camera_target_event(
		const vec3 &_new_target
	) :
		new_target(_new_target)
	{}
};

struct set_camera_pos_event {
	const vec3 &new_pos;

	set_camera_pos_event(
		const vec3 &_new_pos
	) :
		new_pos(_new_pos)
	{}
};

using fcad_event_bus = event_bus<
	fcad_start_event,
	window_input_event,
	set_mode_event,
	mode_switch_event,
	command_cancel_event,
	command_input_event,
	command_submit_event,
	new_vertex_event,
	new_edge_event,
	new_face_event,
	camera_move_event,
	set_camera_target_event,
	set_camera_pos_event
>;

namespace traits {
	template <>
	std::string to_string(const edit_mode &mode, size_t indent);
}
