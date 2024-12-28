#pragma once
#include <stdexcept>

namespace win32 {
	class error : public std::runtime_error {
		using std::runtime_error::runtime_error;
	};
}

std::wstring trim(const std::wstring &s);
