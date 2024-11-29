#pragma once
#include <iostream>
#include "platform/platform.h"
#include "traits.h"

namespace logger {
	extern bool can_use_colors;

	void init();

	template <typename T>
	void info(const T &t, std::ostream &out = std::cout);

	template <typename T>
	void warn(const T &t, std::ostream &out = std::cout);

	template <typename T>
	void error(const T &t, std::ostream &out = std::cout);

	template <typename T>
	void debug(const T &t, std::ostream &out = std::cout);
}

template <typename T>
void logger::info(const T &t, std::ostream &out) {
	if (! platform::stdout_colors_enabled()) {
		out << "[INFO] " + traits::to_string(t) << std::endl;
		return;
	}

	out << "\033[34m[INFO] " + traits::to_string(t) << "\033[0m" << std::endl;
}

template <typename T>
void logger::warn(const T &t, std::ostream &out) {
	if (! platform::stdout_colors_enabled()) {
		out << "[WARN] " + traits::to_string(t) << std::endl;
		return;
	}

	out << "\033[33m[WARN] " + traits::to_string(t) << "\033[0m" << std::endl;
}

template <typename T>
void logger::error(const T &t, std::ostream &out) {
	if (! platform::stdout_colors_enabled()) {
		out << "[ERROR] " + traits::to_string(t) << std::endl;
		return;
	}

	out << "\033[31m[ERROR] " + traits::to_string(t) << "\033[0m" << std::endl;
}

template <typename T>
void logger::debug(const T &t, std::ostream &out) {
	if (! platform::stdout_colors_enabled()) {
		out << "[DEBUG] " + traits::to_string(t) << std::endl;
		return;
	}

	out << "\033[35m[DEBUG] " + traits::to_string(t) << "\033[0m" << std::endl;
}
