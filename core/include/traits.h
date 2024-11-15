#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace traits {
	template <typename T>
	std::string to_string(const T &t, size_t indent = 0);

	template <typename T>
	concept native_stringifiable = requires(const T & t) {
		{ std::to_string(t) } -> std::convertible_to<std::string>;
	};

	template <typename T>
	concept stringifiable = requires(const T & t) {
		{ traits::to_string(t, 0) } -> std::convertible_to<std::string>;
	} || native_stringifiable<T>;

	template <typename T>
	concept pointer_type = requires(const T t) {
		*t;
	};

	template <native_stringifiable T>
	std::string to_string(const T &t, size_t indent = 0);

	template <stringifiable T>
	std::string to_string(const std::vector<T> &v, size_t indent = 0);

	// T is not const here because making it so seems to provoke a possible bug.
	// The compiler accepts a call to to_string with T=const P *, but instantiates
	// a function with T=P *, and the linker fails to find the correct function.
	// With T non-const, the compiler can deduce T=const P * or T=P * as necessary.
	template <pointer_type T>
	std::string to_string(T const &p, size_t indent = 0);

	template <const size_t N>
	std::string to_string(const char(&s)[N], size_t indent = 0);

	template <>
	std::string to_string(const char * const &s, size_t indent);

	template <>
	std::string to_string(const std::string &s, size_t indent);

	template <>
	std::string to_string(const glm::vec3 &v, size_t indent);

	template <>
	std::string to_string(const glm::vec4 &v, size_t indent);

	template <>
	std::string to_string(const glm::quat &q, size_t indent);

	template <>
	std::string to_string(const glm::mat4 &m, size_t indent);
}

namespace util {
	template <traits::stringifiable T>
	struct named_val {
		const std::string name;
		const T &val;

		named_val(const std::string &_name, const T &_val);
	};

	template <traits::stringifiable ... Arg>
	std::string obj_to_string(const std::string &name, size_t indent, named_val<Arg> ... named_vals);
}

template <traits::native_stringifiable T>
std::string traits::to_string(const T &t, size_t) {
	return std::to_string(t);
}

template <traits::stringifiable T>
std::string traits::to_string(const std::vector<T> &v, size_t indent) {
	if (v.empty()) {
		return "[]";
	}

	std::stringstream out{};
	out << "[\n";

	for (const T &item : v) {
		out << std::string(indent + 1, '\t') << to_string(item, indent + 1) << "\n";
	}

	out << std::string(indent, '\t') << "]";

	return out.str();
}

template <traits::pointer_type T>
std::string traits::to_string(T const &p, size_t) {
	std::stringstream out{};

	out << std::hex << (uintptr_t)p;

	return out.str();
}

template <const size_t N>
std::string traits::to_string(const char(&s)[N], size_t) {
	return std::string(s);
}

template <traits::stringifiable T>
util::named_val<T>::named_val(const std::string &_name, const T &_val) :
	name(_name), val(_val) {}

template <traits::stringifiable ... Arg>
std::string util::obj_to_string(const std::string &name, size_t indent, named_val<Arg> ... named_vals) {
	std::stringstream out{};
	int num_pairs = 0;
	out << name + "{";

	const auto stringify_pair = [&]<typename T>(const T & arg) {
		if (num_pairs == 0) {
			out << "\n";
		}

		num_pairs++;
		out << std::string(indent + 1, '\t') << arg.name << ": " << traits::to_string(arg.val, indent + 2) << "\n";
	};

	((void)(stringify_pair(named_vals)), ...);

	out << std::string(indent, '\t') << "}";

	return out.str();
}
