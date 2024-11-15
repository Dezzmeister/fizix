#include <string>
#include <Windows.h>
#include "platform/windows.h"

namespace {
	HANDLE std_out = INVALID_HANDLE_VALUE;
	bool can_use_colors{};

	std::string get_last_error(const std::string &method) {
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
}

void platform::enable_stdout_colors() {
	std_out = GetStdHandle(STD_OUTPUT_HANDLE);

	if (std_out == INVALID_HANDLE_VALUE) {
		throw api_error(get_last_error("GetStdHandle"));
	}

	can_use_colors = SetConsoleMode(
		std_out,
		ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING
	);

	if (! can_use_colors) {
		throw api_error(get_last_error("SetConsoleMode"));
	}
}

bool platform::stdout_colors_enabled() {
	return can_use_colors;
}