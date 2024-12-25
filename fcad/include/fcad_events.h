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

using fcad_event_bus = event_bus<
	window_input_event,
	set_mode_event,
	mode_switch_event,
	command_cancel_event,
	command_input_event,
	command_submit_event,
	new_vertex_event,
	new_edge_event,
	new_face_event
>;

namespace traits {
	template <>
	std::string to_string(const edit_mode &mode, size_t indent);
}
