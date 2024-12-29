#pragma once
#define WIN32_LEAN_AND_MEAN
#define UNICODE
#define _UNICODE
#include <platform/platform.h>
#include "fcad_events.h"

class platform_bridge :
	traits::pinned<platform_bridge>,
	public event_listener<mode_switch_event>
{
public:
	platform_bridge(
		fcad_event_bus &_events,
		const platform::state &_platform,
		const platform::window &_main_window,
		mode_controller &_mode
	);

	int handle(mode_switch_event &event) override;

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

	friend LRESULT CALLBACK command_input_proc(
		HWND,
		UINT,
		WPARAM,
		LPARAM,
		UINT_PTR,
		DWORD_PTR
	);

private:
	fcad_event_bus &events;
	mode_controller &mode;
	HWND main_window{};
	HWND statusbar{};
	HWND command_input{};
	std::wstring mode_str{ L"UNKNOWN" };
};