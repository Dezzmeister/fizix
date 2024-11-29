#pragma once
#include <functional>
#include <optional>
#include <string>
#include <vector>
#include "data_formats/ipaddr.h"
#include "traits.h"
#include "util.h"
#include "setup.h"
#include "physics/collision/vclip.h"

#define expect(arg) test::expect_impl((arg), __FILE__, __LINE__)

namespace test {
	template <typename T>
	class invertible {
	public:
		T& naht();

	protected:
		bool is_inverted{};
	};

	template <typename T>
	class conditional {
	public:
		T& orr();
		T& annd();

		conditional();
		~conditional();

	protected:
		std::optional<assertion_failure> err{};

		bool is_done() const;

	private:
		bool can_short_circuit{};
	};

	template <typename ActualType>
	class matchers :
		public invertible<matchers<ActualType>>,
		public conditional<matchers<ActualType>>
	{
	public:
		matchers(const ActualType &_actual, const char * const _file, int _line) :
			invertible<matchers<ActualType>>(),
			conditional<matchers<ActualType>>(),
			actual(_actual),
			file(_file),
			line(_line)
		{}

		auto to_be(const ActualType &expected) -> decltype(*this)& {
			using traits::to_string;

			if (this->is_done()) {
				return *this;
			}

			bool is_match = actual == expected;

			if (this->is_inverted && is_match) {
				std::string message = "Expected objects to be unequal\nExpected/Actual: " +
					to_string(actual);

				this->err = assertion_failure(
					message,
					file,
					line
				);
			} else if (! this->is_inverted && ! is_match) {
				std::string message = "Expected objects to be equal\nExpected: " +
					to_string(expected) +
					"\nActual: " + to_string(actual);

				this->err = assertion_failure(
					message,
					file,
					line
				);
			}

			return *this;
		}

	private:
		const ActualType &actual;
		const char * const file;
		const int line;
	};

	template <util::numeric ActualType>
	class matchers<ActualType> :
		public invertible<matchers<ActualType>>,
		public conditional<matchers<ActualType>>
	{
	public:
		matchers(const ActualType _actual, const char * const _file, int _line);

		auto to_be(const ActualType expected) -> decltype(*this)&;

	private:
		const ActualType actual;
		const char * const file;
		const int line;
	};

	template <>
	class matchers<std::string> :
		public invertible<matchers<std::string>>,
		public conditional<matchers<std::string>>
	{
	public:
		matchers(const std::string &_actual, const char * const _file, int _line);

		auto to_be(const std::string &expected) -> decltype(*this)&;
		auto to_contain(const std::string &substring) -> decltype(*this)&;
		auto to_have_size(size_t size) -> decltype(*this)&;

	private:
		const std::string actual;
		const char * const file;
		const int line;
	};

	template <>
	class matchers<std::wstring> :
		public invertible<matchers<std::wstring>>,
		public conditional<matchers<std::wstring>>
	{
	public:
		matchers(const std::wstring &_actual, const char * const _file, int _line);

		auto to_be(const std::wstring &expected) -> decltype(*this)&;
		auto to_contain(const std::wstring &substring) -> decltype(*this)&;
		auto to_have_size(size_t size) -> decltype(*this)&;

	private:
		const std::wstring actual;
		const char * const file;
		const int line;
	};

	template <typename T>
	class matchers<std::vector<T>> :
		public invertible<matchers<std::vector<T>>>,
		public conditional<matchers<std::vector<T>>>
	{
	public:
		matchers(const std::vector<T> &_actual, const char * const _file, int _line);

		auto to_have_item(const T &item) -> decltype(*this)&;
		auto to_have_size(size_t size) -> decltype(*this)&;

	private:
		const std::vector<T> &actual;
		const char * const file;
		const int line;
	};

	template <std::ranges::range R>
	class matchers<R> :
		public matchers<std::vector<std::ranges::range_value_t<R>>>
	{
		using value_t = std::ranges::range_value_t<R>;
	public:
		matchers(R &r, const char * const _file, int _line) :
			matchers<std::vector<value_t>>(
				vec,
				_file,
				_line
			),
			vec(r | std::ranges::to<std::vector<value_t>>())
		{}

	private:
		std::vector<value_t> vec{};
	};

	template <typename T>
	class matchers<std::optional<T>> :
		public invertible<matchers<std::optional<T>>>,
		public conditional<matchers<std::optional<T>>>
	{
	public:
		matchers(const std::optional<T> &_actual, const char * const _file, int _line);

		auto to_have_value(const T &expected) -> decltype(*this)&;
		auto to_be_empty() -> decltype(*this)&;

	private:
		const std::optional<T> &actual;
		const char * const file;
		int line;
	};

	template <typename Input>
	using expect_impl = matchers<Input>;

	extern std::optional<assertion_failure> prev_err;
}

template <typename T>
T& test::invertible<T>::naht() {
	is_inverted = ! is_inverted;

	return *static_cast<T*>(this);
}

template <typename T>
T& test::conditional<T>::orr() {
	if (! err) {
		can_short_circuit = true;
	}

	err = {};

	return *static_cast<T*>(this);
}

template <typename T>
T& test::conditional<T>::annd() {
	if (err) {
		throw err.value();
	}

	return *static_cast<T*>(this);
}

template <typename T>
test::conditional<T>::conditional() {
	if (prev_err) {
		const assertion_failure err_out = prev_err.value();
		prev_err = {};

		throw err_out;
	}
}

template <typename T>
test::conditional<T>::~conditional() {
	if (err) {
		prev_err = err;
	}
}

template <typename T>
bool test::conditional<T>::is_done() const {
	return can_short_circuit;
}

template <util::numeric ActualType>
test::matchers<ActualType>::matchers(
	const ActualType _actual,
	const char * const _file,
	int _line
) :
	actual(_actual),
	file(_file),
	line(_line)
{}

template <util::numeric ActualType>
auto test::matchers<ActualType>::to_be(const ActualType expected) -> decltype(*this)& {
	using traits::to_string;

	if (this->is_done()) {
		return *this;
	}

	bool is_match = actual == expected;

	if (this->is_inverted && is_match) {
		this->err = assertion_failure(
			"Expected " + to_string(actual) + " not to be " + to_string(expected),
			file,
			line
		);
	} else if (! this->is_inverted && ! is_match) {
		this->err = assertion_failure(
			"Expected " + to_string(actual) + " to be " + to_string(expected),
			file,
			line
		);
	}

	return *this;
}

template <typename T>
test::matchers<std::vector<T>>::matchers(
	const std::vector<T> &_actual,
	const char * const _file,
	int _line
) :
	actual(_actual),
	file(_file),
	line(_line)
{}

template <typename T>
auto test::matchers<std::vector<T>>::to_have_item(const T &item) -> decltype(*this)& {
	using traits::to_string;

	if (this->is_done()) {
		return *this;
	}

	bool was_found = std::find(std::begin(actual), std::end(actual), item) != std::end(actual);

	if (this->is_inverted && was_found) {
		std::string message = "Expected not to find item in vector\nItem: " +
			to_string(item);

		this->err = assertion_failure(
			message,
			file,
			line
		);
	} else if (! this->is_inverted && ! was_found) {
		std::string message = "Expected to find item in vector\nItem: " +
			to_string(item);

		if (actual.size() <= 4) {
			message += "\nVector: " + to_string(actual);
		}

		this->err = assertion_failure(
			message,
			file,
			line
		);
	}

	return *this;
}

template <typename T>
auto test::matchers<std::vector<T>>::to_have_size(size_t size) -> decltype(*this)& {
	using traits::to_string;

	if (this->is_done()) {
		return *this;
	}

	bool has_size = actual.size() == size;

	if (this->is_inverted && has_size) {
		this->err = assertion_failure(
			"Expected vector not to have size " + to_string(size),
			file,
			line
		);
	} else if (! this->is_inverted && ! has_size) {
		this->err = assertion_failure(
			"Expected vector to have size " + to_string(size) +
			", actual size is " + to_string(actual.size()),
			file,
			line
		);
	}

	return *this;
}

template <typename T>
test::matchers<std::optional<T>>::matchers(
	const std::optional<T> &_actual,
	const char * const _file,
	int _line
) :
	actual(_actual),
	file(_file),
	line(_line)
{}

template <typename T>
auto test::matchers<std::optional<T>>::to_have_value(const T &expected) -> decltype(*this)& {
	using traits::to_string;

	if (this->is_done()) {
		return *this;
	}

	if (this->is_inverted) {
		if (actual && expected == *actual) {
			std::string message = "Expected optional not to have a specific value:\n" +
				to_string(expected);

			this->err = assertion_failure(
				message,
				file,
				line
			);
		}
	} else {
		if (actual) {
			if (expected != *actual) {
				std::string message = "Expected optional to have a specific value\nExpected: " +
					to_string(expected) +
					"\nActual: " + to_string(actual.value());

				this->err = assertion_failure(
					message,
					file,
					line
				);
			}
		} else {
			std::string message = "Expected optional to have a specific value, but it was empty\nExpected: " +
				to_string(expected);

			this->err = assertion_failure(
				message,
				file,
				line
			);
		}
	}

	return *this;
}

template <typename T>
auto test::matchers<std::optional<T>>::to_be_empty() -> decltype(*this)& {
	using traits::to_string;

	if (this->is_done()) {
		return *this;
	}

	if (this->is_inverted && ! actual) {
		this->err = assertion_failure(
			"Expected optional not to be empty",
			file,
			line
		);
	} else if (! this->is_inverted && actual) {
		std::string message = "Expected optional to be empty\nValue: " +
			to_string(actual.value());

		this->err = assertion_failure(
			message,
			file,
			line
		);
	}

	return *this;
}
