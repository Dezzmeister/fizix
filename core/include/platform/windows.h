#pragma once
#include <stdexcept>

namespace platform {
	class api_error : public std::runtime_error {
		using std::runtime_error::runtime_error;
	};

	void enable_stdout_colors();
	bool stdout_colors_enabled();
}