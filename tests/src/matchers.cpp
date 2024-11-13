#include <Windows.h>
#include "matchers.h"
#include "util.h"

std::optional<test::assertion_failure> test::prev_err{};

namespace {
	std::string wstr_to_str(const std::wstring &wstr) {
		static char str_out[1 << 16];

		if (wstr.size() >= util::c_arr_size(str_out)) {
			throw std::runtime_error("Input string is too large");
		}

		int result = WideCharToMultiByte(
			CP_UTF8,
			0,
			wstr.data(),
			(int)wstr.size(),
			str_out,
			(int)(util::c_arr_size(str_out) / sizeof(char)),
			NULL,
			NULL
		);

		if (! result) {
			int error = GetLastError();

			throw std::runtime_error("WideCharToMultiByte() failed with error code " + std::to_string(error));
		}

		return std::string(str_out);
	}
}

test::matchers<std::string>::matchers(const std::string &_actual, const char * const _file, int _line) :
	invertible<matchers<std::string>>(),
	actual(_actual),
	file(_file),
	line(_line)
{}

auto test::matchers<std::string>::to_be(const std::string &expected) -> decltype(*this)& {
	if (this->is_done()) {
		return *this;
	}

	bool is_match = actual == expected;

	if (is_inverted && is_match) {
		this->err = assertion_failure(
			"Expected \"" + actual + "\" not to be equal to \"" + expected + "\"",
			file,
			line
		);
	} else if (! is_inverted && ! is_match) {
		this->err = assertion_failure(
			"Expected \"" + actual + "\" to be equal to \"" + expected + "\"",
			file,
			line
		);
	}

	return *this;
}

auto test::matchers<std::string>::to_contain(const std::string &substring) -> decltype(*this)& {
	if (this->is_done()) {
		return *this;
	}

	bool is_match = actual.find(substring) != std::string::npos;

	if (is_inverted && is_match) {
		this->err = assertion_failure(
			"Expected \"" + actual + "\" not to contain \"" + substring + "\"",
			file,
			line
		);
	} else if (! is_inverted && ! is_match) {
		this->err = assertion_failure(
			"Expected \"" + actual + "\" to contain \"" + substring + "\"",
			file,
			line
		);
	}

	return *this;
}

auto test::matchers<std::string>::to_have_size(size_t size) -> decltype(*this)& {
	if (this->is_done()) {
		return *this;
	}

	bool is_match = actual.size() == size;

	if (is_inverted && is_match) {
		this->err = assertion_failure(
			"Expected \"" + actual + "\" not to have size \"" + std::to_string(size) +
			", actual size is " + std::to_string(actual.size()),
			file,
			line
		);
	} else if (! is_inverted && ! is_match) {
		this->err = assertion_failure(
			"Expected \"" + actual + "\" to have size \"" + std::to_string(size) +
			", actual size is " + std::to_string(actual.size()),
			file,
			line
		);
	}

	return *this;
}

test::matchers<std::wstring>::matchers(const std::wstring &_actual, const char * const _file, int _line) :
	actual(_actual),
	file(_file),
	line(_line)
{}

auto test::matchers<std::wstring>::to_be(const std::wstring &expected) -> decltype(*this)& {
	if (this->is_done()) {
		return *this;
	}

	bool is_match = actual == expected;

	if (is_inverted && is_match) {
		this->err = assertion_failure(
			"Expected \"" + wstr_to_str(actual) + "\" not to be equal to \"" + wstr_to_str(expected) + "\"",
			file,
			line
		);
	} else if (! is_inverted && ! is_match) {
		this->err = assertion_failure(
			"Expected \"" + wstr_to_str(actual) + "\" to be equal to \"" + wstr_to_str(expected) + "\"",
			file,
			line
		);
	}

	return *this;
}

auto test::matchers<std::wstring>::to_contain(const std::wstring &substring) -> decltype(*this)& {
	if (this->is_done()) {
		return *this;
	}

	bool is_match = actual.find(substring) != std::wstring::npos;

	if (is_inverted && is_match) {
		this->err = assertion_failure(
			"Expected \"" + wstr_to_str(actual) + "\" not to contain \"" + wstr_to_str(substring) + "\"",
			file,
			line
		);
	} else if (! is_inverted && ! is_match) {
		this->err = assertion_failure(
			"Expected \"" + wstr_to_str(actual) + "\" to contain \"" + wstr_to_str(substring) + "\"",
			file,
			line
		);
	}

	return *this;
}

auto test::matchers<std::wstring>::to_have_size(size_t size) -> decltype(*this)& {
	if (this->is_done()) {
		return *this;
	}

	bool is_match = actual.size() == size;

	if (is_inverted && is_match) {
		this->err = assertion_failure(
			"Expected \"" + wstr_to_str(actual) + "\" not to have size \"" + std::to_string(size),
			file,
			line
		);
	} else if (! is_inverted && ! is_match) {
		this->err = assertion_failure(
			"Expected \"" + wstr_to_str(actual) + "\" to have size \"" + std::to_string(size) +
			", actual size is " + std::to_string(actual.size()),
			file,
			line
		);
	}

	return *this;
}
