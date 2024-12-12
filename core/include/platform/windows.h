#pragma once
// TODO: Include Windows.h in one place
#define WIN32_LEAN_AND_MEAN
#include <bitset>
#include <Windows.h>
#include "traits.h"

#define KEY_A				0x41
#define KEY_B				0x42
#define KEY_C				0x43
#define KEY_D				0x44
#define KEY_E				0x45
#define KEY_F				0x46
#define KEY_G				0x47
#define KEY_H				0x48
#define KEY_I				0x49
#define KEY_J				0x4a
#define KEY_K				0x4b
#define KEY_L				0x4c
#define KEY_M				0x4d
#define KEY_N				0x4e
#define KEY_O				0x4f
#define KEY_P				0x50
#define KEY_Q				0x51
#define KEY_R				0x52
#define KEY_S				0x53
#define KEY_T				0x54
#define KEY_U				0x55
#define KEY_V				0x56
#define KEY_W				0x57
#define KEY_X				0x58
#define KEY_Y				0x59
#define KEY_Z				0x5a
#define KEY_SHIFT			VK_SHIFT
#define KEY_ESC				VK_ESCAPE
#define KEY_LEFT			VK_LEFT
#define KEY_UP				VK_UP
#define KEY_RIGHT			VK_RIGHT
#define KEY_DOWN			VK_DOWN
#define KEY_PERIOD			VK_DECIMAL
#define KEY_MAX				0x100
#define KEY_MASK			0xFF

#define MOUSE_LEFT			0x00
#define MOUSE_RIGHT			0x01
#define MOUSE_MIDDLE		0x02
#define MOUSE_MAX			0x100
#define MOUSE_MASK			0xFF

namespace platform {
	class window;

	class state : traits::pinned<state> {
	public:
		state();
		// We don't need to define any other special member functions
		// because the class is pinned
		~state();

		HINSTANCE h_inst() const;
		ATOM default_window_class() const;

	private:
		HINSTANCE _h_inst{};
		WNDCLASSEXW default_wc_spec{};
		ATOM default_wc{};
	};

	struct dimensions {
		int width{};
		int height{};
	};

	struct cursor {
		long delta_x{};
		long delta_y{};
		short delta_vscroll{};
		short delta_hscroll{};
		bool is_locked{};
	};

	struct pos {
		long x{};
		long y{};
	};

	// Pinned because each WIN32 window keeps a pointer
	// to its owning platform::window in its GWLP_USERDATA
	// TODO: Send input events from the window and remove the extra
	// "controller" classes
	class window : traits::pinned<window> {
	public:
		window(
			const state &platform_state,
			int width,
			int height,
			const wchar_t * title,
			bool gl = true
		);

		HWND hwnd() const;
		bool has_gl_context() const;
		HGLRC gl_context() const;

		void show() const;
		void hide() const;
		void run(const std::function<void(window&)> &do_frame);
		void destroy();

		dimensions get_window_size() const;

		void make_gl_context_current() const;
		void swap_buffers() const;

		bool is_key_down(int key) const;
		bool is_mouse_btn_down(int btn) const;

		cursor get_cursor() const;
		void lock_cursor();
		void unlock_cursor();
		void reset_cursor_deltas();

		friend LRESULT CALLBACK default_wnd_proc(HWND, UINT, WPARAM, LPARAM);

	private:
		HWND _hwnd{};
		HGLRC glc{};
		HDC dc{};
		RAWINPUTDEVICE input_devices[1];
		char input_buf[2048]{};
		mutable std::bitset<KEY_MAX> keys{};
		mutable std::bitset<KEY_MAX> keys_needs_polling{};
		std::bitset<KEY_MAX> keys_next{};
		mutable std::bitset<MOUSE_MAX> mouse_btns{};
		mutable std::bitset<MOUSE_MAX> mouse_btns_needs_polling{};
		std::bitset<MOUSE_MAX> mouse_btns_next{};
		dimensions window_size{};
		// This position will be unused if Windows never sends
		// MOUSE_MOVE_ABSOLUTE events
		pos curr_mouse{};
		cursor mouse{};
		mutable bool has_been_shown{};
		bool destroyed{};

		void check_has_gl_context() const;
		void check_has_dc() const;
		void destroy_gl_context() const;

		void set_key_down(int key);
		void set_key_up(int key);

		void set_mouse_btn_down(int btn);
		void set_mouse_btn_up(int btn);

		void write_window_size(int width, int height);

		void handle_raw_mouse();
	};
}