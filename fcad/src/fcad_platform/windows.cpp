#include "fcad_platform/platform.h"
#include <CommCtrl.h>
#include "controllers/mode.h"

const int mode_section_width = 256;

LRESULT CALLBACK statusbar_proc(
	HWND hwnd,
	UINT message,
	WPARAM w_param,
	LPARAM l_param,
	UINT_PTR instance_id,
	DWORD_PTR ref_data
) {
	UNREFERENCED_PARAMETER(instance_id);
	platform_bridge * bridge = (platform_bridge *)ref_data;

	switch (message) {
		case WM_COMMAND: {
			if ((HWND)l_param == bridge->command_input && HIWORD(w_param) == EN_CHANGE) {
				SetLastError(0);
				int num_chars = GetWindowTextLengthW(bridge->command_input);

				if (! num_chars && GetLastError()) {
					throw platform::api_error(
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
	platform_bridge * bridge = (platform_bridge *)ref_data;

	switch (message) {
		case WM_CHAR: {
			if (bridge->is_cue_banner_set) {
				SendMessageW(hwnd, EM_SETCUEBANNER, FALSE, (LPARAM)L"");
				bridge->is_cue_banner_set = false;
			}

			if (w_param != VK_ESCAPE && w_param != VK_RETURN) {
				break;
			}

			SetLastError(0);
			int num_chars = GetWindowTextLengthW(hwnd);

			if (! num_chars && GetLastError()) {
				throw platform::api_error(
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

			bridge->mode.set_mode(edit_mode::Normal);

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
	platform_bridge * bridge = (platform_bridge *)ref_data;
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

platform_bridge::platform_bridge(
	fcad_event_bus &_events,
	const platform::state &_platform,
	const platform::window &_main_window,
	mode_controller &_mode
) :
	event_listener<mode_switch_event>(&_events),
	events(_events),
	mode(_mode),
	main_window(_main_window.hwnd())
{
	event_listener<mode_switch_event>::subscribe();

	INITCOMMONCONTROLSEX ctrls = {
		.dwSize = sizeof INITCOMMONCONTROLSEX,
		.dwICC = ICC_WIN95_CLASSES
	};

	if (! InitCommonControlsEx(&ctrls)) {
		throw platform::api_error("Failed to initialize common controls");
	}

	statusbar = CreateWindowExW(
		0,
		STATUSCLASSNAMEW,
		(PCWSTR) NULL,
		SBARS_SIZEGRIP | WS_CHILD | WS_VISIBLE,
		0, 0, 0, 0,
		main_window,
		NULL,
		_platform.h_inst(),
		NULL
	);

	if (! statusbar) {
		throw platform::api_error(
			"Failed to create status bar: " +
			platform::win32::get_last_error("CreateWindowExW")
		);
	}

	command_input = CreateWindowExW(
		0,
		L"EDIT",
		NULL,
		WS_BORDER | WS_CHILD | WS_VISIBLE | ES_LEFT,
		0, 0, 0, 0,
		statusbar,
		NULL,
		_platform.h_inst(),
		NULL
	);

	if (! command_input) {
		throw platform::api_error(
			"Failed to create command input edit control: " +
			platform::win32::get_last_error("CreateWindowEx")
		);
	}

	if (! SetWindowSubclass(main_window, main_window_proc, 1, (DWORD_PTR)this)) {
		throw platform::api_error("Failed to set main window subclass proc");
	}

	if (! SetWindowSubclass(command_input, command_input_proc, 1, (DWORD_PTR)this)) {
		throw platform::api_error("Failed to set command input subclass proc");
	}

	if (! SetWindowSubclass(statusbar, statusbar_proc, 1, (DWORD_PTR)this)) {
		throw platform::api_error("Failed to set statusbar subclass proc");
	}

	ShowWindow(statusbar, SW_NORMAL);
	ShowWindow(command_input, SW_NORMAL);

	int start_width = 800 - mode_section_width;
	int start_sections[] = { start_width, -1 };
	SendMessageW(statusbar, SB_SETPARTS, util::c_arr_size(start_sections), (LPARAM)&start_sections);
	SendMessageW(statusbar, SB_SETTEXTW, SBT_OWNERDRAW, NULL);
}

void platform_bridge::set_cue_text(const std::wstring &text) const {
	SendMessageW(command_input, EM_SETCUEBANNER, FALSE, (LPARAM)text.data());
	is_cue_banner_set = true;
}

int platform_bridge::handle(mode_switch_event &event) {
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
