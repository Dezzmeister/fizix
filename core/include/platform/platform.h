#pragma once
#include <stdexcept>
#ifdef _WIN32
#include "platform/windows.h"
#endif

namespace platform {
	class api_error : public std::runtime_error {
		using std::runtime_error::runtime_error;
	};

	enum class gpu_preference {
		Default,
		Discrete
	};

	void enable_stdout_colors();
	bool stdout_colors_enabled();
	void set_gpu_preference(gpu_preference pref);
	void enable_fp_exceptions();
}
