#pragma once
#define WIN32_LEAN_AND_MEAN
#define UNICODE
#define _UNICODE
#include <platform/platform.h>
#include "fcad_events.h"
#include "resource.h"

class platform_bridge :
	traits::pinned<platform_bridge>,
	public event_listener<mode_switch_event>
{
public:
	platform_bridge(
		fcad_event_bus &_events,
		platform::state &_platform,
		const platform::window &_main_window,
		mode_controller &_mode
	);

	void set_cue_text(const std::wstring &text) const;
	void set_cue_text(const std::string &text) const;
	void create_help_dialog(const std::string &help_text);
	void destroy_help_dialog();

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

	friend BOOL CALLBACK help_proc(
		HWND,
		UINT,
		WPARAM,
		LPARAM
	);

private:
	fcad_event_bus &events;
	mode_controller &mode;
	platform::state &platform;
	HWND main_window{};
	HWND statusbar{};
	HWND command_input{};
	HWND help_dialog{};
	std::wstring mode_str{ L"UNKNOWN" };
	mutable bool is_cue_banner_set{};
};