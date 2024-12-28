#define WIN32_LEAN_AND_MEAN
#define UNICODE
#define _UNICODE
#include <Windows.h>
#include <CommCtrl.h>
#include <controllers.h>
#include <draw2d.h>
#include <gdi_plus_context.h>
#include <gl.h>
#include <helpers.h>
#include <hardware_constants.h>
#include <logging.h>
#include <mesh.h>
#include <player.h>
#include <shader_store.h>
#include <shapes.h>
#include <texture_store.h>
#include <util.h>
#include <world.h>
#include "action.h"
#include "controllers/action.h"
#include "controllers/camera.h"
#include "controllers/command.h"
#include "controllers/edit_history.h"
#include "fcad_events.h"
#include "controllers/file.h"
#include "controllers/geometry.h"
#include "controllers/mode.h"

const int mode_section_width = 256;

class win32_bridge :
	public event_listener<mode_switch_event>
{
public:
	fcad_event_bus &events;

	win32_bridge(
		fcad_event_bus &_events,
		HWND _main_window,
		HWND _statusbar,
		HWND _command_input
	) :
		event_listener<mode_switch_event>(&_events),
		events(_events),
		main_window(_main_window),
		statusbar(_statusbar),
		command_input(_command_input)
	{
		event_listener<mode_switch_event>::subscribe();
	}

	int handle(mode_switch_event &event) override {
		if (event.new_mode == edit_mode::Command) {
			SetFocus(command_input);

			if (! event.hint.empty()) {
				SetWindowTextW(command_input, event.hint.c_str());
				SendMessageW(command_input, EM_SETSEL, event.hint.size(), event.hint.size());
			}
		} else {
			SetFocus(main_window);
			SetWindowTextW(command_input, L"");
		}

		mode_str = util::to_wstring(traits::to_string(event.new_mode));
		SendMessageW(statusbar, SB_SETTEXTW, 1, (LPARAM)mode_str.c_str());

		return 0;
	}

	friend LRESULT CALLBACK main_window_proc(
		HWND,
		UINT,
		WPARAM,
		LPARAM,
		UINT_PTR,
		DWORD_PTR
	);

	friend LRESULT CALLBACK statusbar_proc(
		HWND,
		UINT,
		WPARAM,
		LPARAM,
		UINT_PTR,
		DWORD_PTR
	);

private:
	HWND main_window{};
	HWND statusbar{};
	HWND command_input{};
	std::wstring mode_str{ L"UNKNOWN" };
};

LRESULT CALLBACK statusbar_proc(
	HWND hwnd,
	UINT message,
	WPARAM w_param,
	LPARAM l_param,
	UINT_PTR instance_id,
	DWORD_PTR ref_data
) {
	UNREFERENCED_PARAMETER(instance_id);
	win32_bridge * bridge = (win32_bridge *)ref_data;

	switch (message) {
		case WM_COMMAND: {
			if ((HWND)l_param == bridge->command_input && HIWORD(w_param) == EN_CHANGE) {
				SetLastError(0);
				int num_chars = GetWindowTextLengthW(bridge->command_input);

				if (! num_chars && GetLastError()) {
					throw win32::error(
						"Failed to get command input's text length: " +
						platform::win32::get_last_error("GetWindowTextLengthW")
					);
				}

				std::wstring command_buf(num_chars, L'\0');
				GetWindowTextW(bridge->command_input, command_buf.data(), num_chars + 1);

				command_input_event command_event(command_buf);
				bridge->events.fire(command_event);

				return 0;
			}

			break;
		}
	}

	return DefSubclassProc(hwnd, message, w_param, l_param);
}

LRESULT CALLBACK command_input_proc(
	HWND hwnd,
	UINT message,
	WPARAM w_param,
	LPARAM l_param,
	UINT_PTR instance_id,
	DWORD_PTR ref_data
) {
	UNREFERENCED_PARAMETER(instance_id);
	win32_bridge * bridge = (win32_bridge *)ref_data;

	switch (message) {
		case WM_CHAR: {
			if (w_param != VK_ESCAPE && w_param != VK_RETURN) {
				break;
			}

			SetLastError(0);
			int num_chars = GetWindowTextLengthW(hwnd);

			if (! num_chars && GetLastError()) {
				throw win32::error(
					"Failed to get command input's text length: " +
					platform::win32::get_last_error("GetWindowTextLengthW")
				);
			}

			std::wstring command_buf(num_chars, L'\0');
			GetWindowTextW(hwnd, command_buf.data(), num_chars + 1);

			if (w_param == VK_ESCAPE) {
				command_cancel_event command_event(command_buf);
				bridge->events.fire(command_event);
			} else {
				assert(w_param == VK_RETURN);

				command_submit_event command_event(command_buf);
				bridge->events.fire(command_event);
			}

			set_mode_event mode_event(edit_mode::Normal);
			bridge->events.fire(mode_event);

			return 0;
		}
	}

	return DefSubclassProc(hwnd, message, w_param, l_param);
}

LRESULT CALLBACK main_window_proc(
	HWND hwnd,
	UINT message,
	WPARAM w_param,
	LPARAM l_param,
	UINT_PTR instance_id,
	DWORD_PTR ref_data
) {
	UNREFERENCED_PARAMETER(instance_id);
	win32_bridge * bridge = (win32_bridge *)ref_data;
	HWND statusbar = bridge->statusbar;
	HWND command_input = bridge->command_input;

	switch (message) {
		case WM_SIZE: {
			int new_width = LOWORD(l_param) - mode_section_width;
			int sections[] = { new_width, -1 };

			SendMessageW(statusbar, WM_SIZE, w_param, l_param);
			SendMessageW(statusbar, SB_SETPARTS, util::c_arr_size(sections), (LPARAM)&sections);

			break;
		}
		case WM_DRAWITEM: {
			DRAWITEMSTRUCT * drawitem = (DRAWITEMSTRUCT *)l_param;
			RECT section_rect = drawitem->rcItem;

			if (drawitem->itemID != 0) {
				break;
			}

			MoveWindow(
				command_input,
				section_rect.left,
				section_rect.top,
				section_rect.right - section_rect.left,
				section_rect.bottom - section_rect.top,
				true
			);

			return TRUE;
		}
		case WM_CHAR: {
			window_input_event event((char)w_param);
			bridge->events.fire(event);

			return 0;
		}
		case WM_LBUTTONDOWN: {
			SetFocus(hwnd);
			break;
		}
	}

	return DefSubclassProc(hwnd, message, w_param, l_param);
}

void create_shape(fcad_event_bus &events) {
	// TODO: macros, replay files
	std::wstring commands[] = {
		L":v 1 1 1",
		L":v 1 1 -1",
		L":v 1 -1 1",
		L":v 1 -1 -1",
		L":v -1 1 1",
		L":v -1 1 -1",
		L":v -1 -1 1",
		L":v -1 -1 -1",
		L":f 0 1 5 4",
		L":f 0 2 3 1",
		L":f 0 4 6 2",
		L":f 7 3 2 6",
		L":f 7 6 4 5",
		L":f 7 5 1 3"
	};

	for (const std::wstring &str : commands) {
		command_submit_event event(str);
		events.fire(event);
	}
}

// TODO: Check command line and possibly load file
int main(int, const char * const * const) {
	INITCOMMONCONTROLSEX ctrls = {
		.dwSize = sizeof INITCOMMONCONTROLSEX,
		.dwICC = ICC_WIN95_CLASSES
	};

	if (! InitCommonControlsEx(&ctrls)) {
		throw win32::error("Failed to initialize common controls");
	}

	logger::init();
	platform::state platform_state{};
	platform::window main_window(platform_state, 800, 800, L"FCAD");

	HWND statusbar = CreateWindowExW(
		0,
		STATUSCLASSNAMEW,
		(PCWSTR) NULL,
		SBARS_SIZEGRIP | WS_CHILD | WS_VISIBLE,
		0, 0, 0, 0,
		main_window.hwnd(),
		NULL,
		platform_state.h_inst(),
		NULL
	);

	if (! statusbar) {
		throw win32::error(
			"Failed to create status bar: " +
			platform::win32::get_last_error("CreateWindowExW")
		);
	}

	HWND command_input = CreateWindowExW(
		0,
		L"EDIT",
		NULL,
		WS_BORDER | WS_CHILD | WS_VISIBLE | ES_LEFT,
		0, 0, 0, 0,
		statusbar,
		NULL,
		platform_state.h_inst(),
		NULL
	);

	if (! command_input) {
		throw win32::error(
			"Failed to create command input edit control: " +
			platform::win32::get_last_error("CreateWindowEx")
		);
	}

	event_buses buses;
	fcad_event_bus events;
	win32_bridge bridge(events, main_window.hwnd(), statusbar, command_input);
	window_actions actions = make_window_actions(buses, events);
	action_controller ac(events, actions.actions);
	mode_controller modes(buses, events);
	command_controller commands = make_commands(buses, events);
	edit_history_controller edit_history(events);
	file_controller fc(events);

	if (! SetWindowSubclass(main_window.hwnd(), main_window_proc, 1, (DWORD_PTR)&bridge)) {
		logger::error("Failed to set main window subclass proc");
		return 1;
	}

	if (! SetWindowSubclass(command_input, command_input_proc, 1, (DWORD_PTR)&bridge)) {
		logger::error("Failed to set command input subclass proc");
		return 1;
	}

	if (! SetWindowSubclass(statusbar, statusbar_proc, 1, (DWORD_PTR)&bridge)) {
		logger::error("Failed to set statusbar subclass proc");
		return 1;
	}

	ShowWindow(statusbar, SW_NORMAL);
	ShowWindow(command_input, SW_NORMAL);

	int start_width = 800 - mode_section_width;
	int start_sections[] = { start_width, -1 };
	SendMessageW(statusbar, SB_SETPARTS, util::c_arr_size(start_sections), (LPARAM)&start_sections);
	SendMessageW(statusbar, SB_SETTEXTW, SBT_OWNERDRAW, NULL);

	gdi_plus_context gdi_plus;
	main_window.show();
	main_window.make_gl_context_current();

	glViewport(0, 0, 800, 800);
	// Old Windows BSOD blue
	glClearColor(0.031f, 0.152f, 0.961f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	shapes::init();

	// TODO: Custom camera
	hardware_constants hw_consts(buses);

	program_start_event program_start(&main_window);
	pre_render_pass_event pre_render_event(&main_window, &hw_consts);
	// TODO: Make built-in shaders and textures optional
	shader_store shaders(buses);
	texture_store textures(buses);
	renderer2d draw2d(buses);
	draw_event draw_event_inst(&main_window, shaders, textures);
	post_processing_event post_processing_event_inst(
		&main_window,
		shaders,
		textures,
		draw2d
	);
	post_render_pass_event post_render_event{};

	key_controller keys(buses, {
		KEY_ESC,
		KEY_CTRL,
		KEY_H,
		KEY_J,
		KEY_K,
		KEY_L,
		KEY_I,
		KEY_O,
		KEY_T,
		KEY_X,
		KEY_Y,
		KEY_Z
	});
	// TODO: Disable mouse locking
	mouse_controller mouse(buses, {}, KEY_ESC);
	screen_controller screen(buses);
	camera_controller camera(buses, events);
	geometry_controller geom(buses, events);
	fcad_start_event fcad_start(geom, edit_history, fc);

	buses.lifecycle.fire(program_start);
	events.fire(fcad_start);

	create_shape(events);

	platform::run([&]() {
		buses.render.fire(pre_render_event);
		buses.render.fire(draw_event_inst);
		buses.render.fire(post_processing_event_inst);

		main_window.swap_buffers();

		buses.render.fire(post_render_event);
	});

	program_stop_event program_stop(0);
	buses.lifecycle.fire(program_stop);

	return 0;
}