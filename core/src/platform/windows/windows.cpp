#ifdef _WIN32
#include <string>
#include <Windows.h>
#include <windowsx.h>
#include "logging.h"
#include "platform/platform.h"
#include "unique_handle.h"
#include "util.h"

extern "C" {
	__declspec(dllexport) DWORD NvOptimusEnablement = 0;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 0;

	// This is a special symbol that the linker will resolve
	// to the PE image's base address.
	// See https://devblogs.microsoft.com/oldnewthing/20041025-00/?p=37483
	extern IMAGE_DOS_HEADER __ImageBase;
}

namespace {
	HANDLE std_out = INVALID_HANDLE_VALUE;
	bool can_use_colors{};

	unique_handle<HDC> get_hdc(HWND hwnd) {
		return unique_handle<HDC>(
			nullptr,
			GetDC(hwnd),
			[=](const HDC hdc) {
				ReleaseDC(hwnd, hdc);
			}
		);
	}
}

LRESULT CALLBACK platform::default_wnd_proc(
	HWND hwnd,
	UINT message,
	WPARAM w_param,
	LPARAM l_param
) {
	LONG_PTR user_lp = GetWindowLongPtrW(hwnd, GWLP_USERDATA);

	if (! user_lp) {
		if (GetLastError()) {
			logger::error(
				"Failed to get window userdata: " +
				win32::get_last_error("GetWindowLongPtrW")
			);
		}

		// There will be no user data when the window is created; the window
		// receives a few messages before CreateWindowExW returns
		return DefWindowProcW(hwnd, message, w_param, l_param);
	}

	platform::window * win = (platform::window *)user_lp;

	switch (message) {
		case WM_DESTROY: {
			PostQuitMessage(0);
			break;
		}
		case WM_KEYDOWN: {
			win->set_key_down((int)w_param);
			break;
		}
		case WM_KEYUP: {
			win->set_key_up((int)w_param);
			break;
		}
		case WM_LBUTTONDOWN: {
			win->set_mouse_btn_down(MOUSE_LEFT);
			break;
		}
		case WM_LBUTTONUP: {
			win->set_mouse_btn_up(MOUSE_LEFT);
			break;
		}
		case WM_MBUTTONDOWN: {
			win->set_mouse_btn_down(MOUSE_MIDDLE);
			break;
		}
		case WM_MBUTTONUP: {
			 win->set_mouse_btn_up(MOUSE_MIDDLE);
			break;
		}
		case WM_RBUTTONDOWN: {
			win->set_mouse_btn_down(MOUSE_RIGHT);
			break;
		}
		case WM_RBUTTONUP: {
			win->set_mouse_btn_up(MOUSE_RIGHT);
			break;
		}
		case WM_MOUSEMOVE: {
			if (win->mouse.is_locked) {
				RECT r;
				if (! GetWindowRect(win->_hwnd, &r)) {
					logger::error(
						"Failed to get window rect: " +
						win32::get_last_error("GetWindowRect")
					);
					break;
				}

				if (! SetCursorPos(r.left + + (r.right - r.left) / 2, r.top + (r.bottom - r.top) / 2)) {
					logger::error(
						"Failed to set cursor pos: " +
						win32::get_last_error("SetCursorPos")
					);
				}
			}

			break;
		}
		case WM_SIZE: {
			int width = LOWORD(l_param);
			int height = HIWORD(l_param);

			win->write_window_size(width, height);

			return DefWindowProcW(hwnd, message, w_param, l_param);
		}
		case WM_INPUT: {
			HRAWINPUT h_input = (HRAWINPUT)l_param;
			UINT data_size = (UINT)util::c_arr_size(win->input_buf);
			UINT num_bytes =
				GetRawInputData(
					h_input,
					RID_INPUT,
					win->input_buf,
					&data_size,
					sizeof RAWINPUTHEADER
				);
			RAWINPUT * input = (RAWINPUT *)win->input_buf;

			if (num_bytes == -1) {
				logger::error(
					"Failed to get raw input data: " +
					win32::get_last_error("GetRawInputData")
				);
				goto input_cleanup;
			}

			if (input->header.dwType == RIM_TYPEMOUSE) {
				win->handle_raw_mouse();
			}

			input_cleanup:
			DefWindowProcW(hwnd, message, w_param, l_param);
			break;
		}
		default: {
			return DefWindowProcW(hwnd, message, w_param, l_param);
		}
	}

	return 0;
}

platform::state::state() :
	_h_inst((HINSTANCE)&__ImageBase)
{
	default_wc_spec.cbSize = sizeof default_wc_spec;
	default_wc_spec.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	default_wc_spec.lpfnWndProc = default_wnd_proc;
	default_wc_spec.hInstance = _h_inst;
	default_wc_spec.hCursor = LoadCursorA(NULL, IDC_ARROW);
	default_wc_spec.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	default_wc_spec.lpszClassName = L"FizixGLWindow";

	if (! (default_wc = RegisterClassExW(&default_wc_spec))) {
		throw api_error(
			"Failed to create window class: " +
			win32::get_last_error("RegisterClassExW")
		);
	}
}

platform::state::~state() {
	if (default_wc) {
		if (! UnregisterClassW(L"FizixGLWindow", _h_inst)) {
			logger::error(
				"Failed to unregister window class: " +
				win32::get_last_error("UnregisterClassW")
			);
		}
	}
}

HINSTANCE platform::state::h_inst() const {
	return _h_inst;
}

ATOM platform::state::default_window_class() const {
	return default_wc;
}

platform::window::window(
	const state &platform_state,
	int width,
	int height,
	const wchar_t * const title,
	bool gl
) {
	// TODO: DPI-aware
	_hwnd = CreateWindowExW(
		WS_EX_OVERLAPPEDWINDOW,
		(LPCWSTR)platform_state.default_window_class(),
		title,
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, width, height,
		NULL, NULL,
		platform_state.h_inst(),
		NULL
	);

	if (! _hwnd) {
		throw api_error(
			"Failed to create window: " +
			win32::get_last_error("CreateWindowExW")
		);
	}

	SetLastError(0);

	if (! SetWindowLongPtrW(_hwnd, GWLP_USERDATA, (LONG_PTR)this) && GetLastError()) {
		throw api_error(
			"Failed to attach user data to window: " +
			win32::get_last_error("SetWindowLongPtr")
		);
	}

	RECT window_rect{};
	if (! GetWindowRect(_hwnd, &window_rect)) {
		throw api_error(
			"Failed to get window size: " +
			win32::get_last_error("GetWindowRect")
		);
	}

	window_size.width = window_rect.left - window_rect.right;
	window_size.height = window_rect.top - window_rect.bottom;

	input_devices[0] = {
		// https://learn.microsoft.com/en-us/windows-hardware/drivers/hid/hid-architecture#hid-clients-supported-in-windows
		.usUsagePage = 0x0001,
		.usUsage = 0x0002,
		.dwFlags = 0,
		.hwndTarget = _hwnd
	};

	if (! RegisterRawInputDevices(
		input_devices,
		(UINT)util::c_arr_size(input_devices),
		sizeof RAWINPUTDEVICE
	)) {
		throw api_error(
			"Failed to register raw input devices: " +
			win32::get_last_error("RegisterRawInputDevices")
		);
	}

	if (! gl) {
		return;
	}

	PIXELFORMATDESCRIPTOR pfd{};
	pfd.nSize = sizeof PIXELFORMATDESCRIPTOR;
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 8;
	pfd.iLayerType = PFD_MAIN_PLANE;

	{
		unique_handle<HDC> hdc = get_hdc(_hwnd);
		int pixel_format = ChoosePixelFormat(hdc, &pfd);

		if (! pixel_format) {
			throw api_error(
				"Failed to choose a suitable pixel format: " +
				win32::get_last_error("ChoosePixelFormat")
			);
		}

		if (! SetPixelFormat(hdc, pixel_format, &pfd)) {
			throw api_error(
				"Failed to set pixel format: " +
				win32::get_last_error("SetPixelFormat")
			);
		}

		// TODO: Trampoline context?
		glc = wglCreateContext(hdc);

		if (! glc) {
			throw api_error(
				"Failed to create OpenGL context: " +
				win32::get_last_error("wglCreateContext")
			);
		}
	}

	dc = GetDC(_hwnd);
}

platform::window::~window() {
	try {
		if (has_gl_context()) {
			destroy_gl_context();
		}

		if (dc) {
			ReleaseDC(_hwnd, dc);
		}
	} catch (const api_error &err) {
		logger::error(
			"Failed to destroy window: " +
			std::string(err.what())
		);
	}
}

HWND platform::window::hwnd() const {
	return _hwnd;
}

bool platform::window::has_gl_context() const {
	return glc;
}

HGLRC platform::window::gl_context() const {
	check_has_gl_context();

	return glc;
}

void platform::window::show() const {
	int show_cmd = has_been_shown ? SW_SHOW : SW_NORMAL;

	ShowWindow(_hwnd, show_cmd);

	has_been_shown = true;
}

void platform::window::hide() const {
	ShowWindow(_hwnd, SW_HIDE);
}

void platform::window::make_gl_context_current() const {
	check_has_gl_context();
	check_has_dc();

	if (! wglMakeCurrent(dc, glc)) {
		throw api_error(
			"Failed to make OpenGL context current: " +
			win32::get_last_error("wglMakeCurrent")
		);
	}
}

void platform::window::swap_buffers() const {
	check_has_gl_context();
	check_has_dc();

	SwapBuffers(dc);
}

bool platform::window::is_key_down(int key) const {
	int key_idx = key & KEY_MASK;
	bool out = keys[key_idx];

	if (keys_needs_polling[key_idx]) {
		keys[key_idx] = keys_next[key_idx];
		keys_needs_polling[key_idx] = false;
	}

	return out;
}

void platform::window::set_key_down(int key) {
	int key_idx = key & KEY_MASK;

	if (! keys_needs_polling[key_idx]) {
		keys[key_idx] = true;
	}

	keys_next[key_idx] = true;
	keys_needs_polling[key_idx] = true;
}

void platform::window::set_key_up(int key) {
	int key_idx = key & KEY_MASK;

	if (! keys_needs_polling[key_idx]) {
		keys[key_idx] = false;
	}

	keys_next[key_idx] = false;
	keys_needs_polling[key_idx] = true;
}

bool platform::window::is_mouse_btn_down(int btn) const {
	int mouse_idx = btn & MOUSE_MASK;
	bool out = mouse_btns[mouse_idx];

	if (mouse_btns_needs_polling[mouse_idx]) {
		mouse_btns[mouse_idx] = mouse_btns_next[mouse_idx];
		mouse_btns_needs_polling[mouse_idx] = false;
	}

	return out;
}

void platform::window::set_mouse_btn_down(int btn) {
	int mouse_idx = btn & MOUSE_MASK;

	if (! mouse_btns_needs_polling[mouse_idx]) {
		mouse_btns[mouse_idx] = true;
	}

	mouse_btns_next[mouse_idx] = true;
	mouse_btns_needs_polling[mouse_idx] = true;
}

void platform::window::set_mouse_btn_up(int btn) {
	int mouse_idx = btn & MOUSE_MASK;

	if (! mouse_btns_needs_polling[mouse_idx]) {
		mouse_btns[mouse_idx] = false;
	}

	mouse_btns_next[mouse_idx] = false;
	mouse_btns_needs_polling[mouse_idx] = true;
}

platform::dimensions platform::window::get_window_size() const {
	return window_size;
}

platform::cursor platform::window::get_cursor() const {
	return mouse;
}

void platform::window::lock_cursor() {
	mouse.is_locked = true;
	ShowCursor(false);
}

void platform::window::unlock_cursor() {
	mouse.is_locked = false;
	ShowCursor(true);
}

void platform::window::reset_cursor_deltas() {
	mouse.delta_x = 0;
	mouse.delta_y = 0;
	mouse.delta_vscroll = 0;
	mouse.delta_hscroll = 0;
}

void platform::window::destroy_gl_context() const {
	check_has_gl_context();

	if (! wglDeleteContext(glc)) {
		throw api_error(
			"Failed to delete OpenGL context: " +
			win32::get_last_error("wglDeleteContext")
		);
	}
}

void platform::window::check_has_gl_context() const {
	if (! has_gl_context()) {
		throw api_error(
			"Attempted to call a GL function on a window "
			"without an OpenGL context"
		);
	}
}

void platform::window::check_has_dc() const {
	if (! dc) {
		throw api_error(
			"Attempted to call a function that requires a device context"
		);
	}
}

void platform::window::write_window_size(int width, int height) {
	window_size.width = width;
	window_size.height = height;
}

void platform::window::handle_raw_mouse() {
	RAWINPUT * input = (RAWINPUT *)input_buf;

	if (input->data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE) {
		mouse.delta_x = input->data.mouse.lLastX - curr_mouse.x;
		mouse.delta_y = input->data.mouse.lLastY - curr_mouse.y;
		curr_mouse.x = input->data.mouse.lLastX;
		curr_mouse.y = input->data.mouse.lLastY;
	} else {
		mouse.delta_x = input->data.mouse.lLastX;
		mouse.delta_y = input->data.mouse.lLastY;
	}

	if (input->data.mouse.usButtonFlags & RI_MOUSE_WHEEL) {
		mouse.delta_vscroll = (short)input->data.mouse.usButtonData;
	} else if (input->data.mouse.usButtonFlags & RI_MOUSE_HWHEEL) {
		mouse.delta_hscroll = (short)input->data.mouse.usButtonData;
	}
}

void platform::run(const std::function<void(void)> &do_frame) {
	MSG msg;
	BOOL msg_result;

	while (true) {
		while (PeekMessageW(&msg, NULL, 0, 0, PM_NOREMOVE)) {
			msg_result = GetMessageW(&msg, NULL, 0, 0);

			if (msg_result == 0) {
				return;
			} else if (msg_result == -1) {
				throw api_error(
					"Failed to get message on message queue: " +
					win32::get_last_error("GetMessageW")
				);
			} else {
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}
		}

		do_frame();
	}
}

void platform::enable_stdout_colors() {
	std_out = GetStdHandle(STD_OUTPUT_HANDLE);

	if (std_out == INVALID_HANDLE_VALUE) {
		throw api_error(win32::get_last_error("GetStdHandle"));
	}

	can_use_colors = SetConsoleMode(
		std_out,
		ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING
	);

	if (! can_use_colors) {
		throw api_error(win32::get_last_error("SetConsoleMode"));
	}
}

bool platform::stdout_colors_enabled() {
	return can_use_colors;
}

void platform::set_gpu_preference(gpu_preference pref) {
	if (pref == gpu_preference::Default) {
		NvOptimusEnablement = 0;
		AmdPowerXpressRequestHighPerformance = 0;
	} else if (pref == gpu_preference::Discrete) {
		NvOptimusEnablement = 1;
		AmdPowerXpressRequestHighPerformance = 1;
	}
}

std::string platform::win32::get_last_error(const std::string &method) {
	DWORD err_code = GetLastError();
	LPSTR buf = NULL;

	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		err_code,
		0,
		(LPSTR)&buf,
		0,
		NULL
	);

	std::string out = "Windows API call (" + method + ") failed: " +
		std::string(buf);
	LocalFree(buf);

	return out;
}
#endif