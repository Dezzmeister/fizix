#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

namespace traits {
	template <typename T>
	std::string to_string(const T &t, size_t indent = 0);

	template <typename T>
	concept native_stringifiable = requires(const T & t) {
		{ std::to_string(t) } -> std::convertible_to<std::string>;
	};

	template <typename T>
	concept pointer_type = std::is_pointer_v<T>;

	template <native_stringifiable T>
	std::string to_string(const T &t, size_t indent = 0);

	template <typename T>
	std::string to_string(const std::vector<T> &v, size_t indent = 0);

	template <pointer_type T>
	std::string to_string(T const &p, size_t indent = 0);

	template <const size_t N>
	std::string to_string(const char(&s)[N], size_t indent = 0);

	template <typename ...Ts>
	std::string to_string(const std::variant<Ts...> &v, size_t indent = 0);

	template <typename T>
	std::string to_string(const std::optional<T> &opt, size_t indent = 0);

	template <>
	std::string to_string(const char * const &s, size_t indent);

	template <>
	std::string to_string(const unsigned char * const &s, size_t indent);

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
	template <typename T>
	struct named_val {
		const std::string name;
		const T &val;

		named_val(const std::string &_name, const T &_val);
	};

	template <typename ... Arg>
	std::string obj_to_string(const std::string &name, size_t indent, named_val<Arg> ... named_vals);
}

template <typename T>
std::string traits::to_string(const T&, size_t) {
	return std::string(typeid(T).name());
}

template <traits::native_stringifiable T>
std::string traits::to_string(const T &t, size_t) {
	return std::to_string(t);
}

template <typename T>
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

template <typename ...Ts>
std::string traits::to_string(const std::variant<Ts...> &v, size_t indent) {
	using traits::to_string;
	using std::get;

	std::string out = "variant(<empty>)";

	const auto stringify_val = [&]<typename T>() {
		if (std::holds_alternative<T>(v)) {
			out = "variant(" + to_string(get<T>(v), indent) + ")";
		}
	};

	(stringify_val.template operator()<Ts>(), ...);

	return out;
}

template <typename T>
std::string traits::to_string(const std::optional<T> &opt, size_t) {
	using traits::to_string;

	if (! opt) {
		return "optional(<empty>)";
	}

	return "optional(" + to_string(*opt) + ")";
}

template <typename T>
util::named_val<T>::named_val(const std::string &_name, const T &_val) :
	name(_name), val(_val) {}

template <typename ...Arg>
std::string util::obj_to_string(const std::string &name, size_t indent, named_val<Arg>... named_vals) {
	using traits::to_string;

	std::stringstream out{};
	int num_pairs = 0;
	out << name + "{";

	const auto stringify_pair = [&]<typename T>(const T &arg) {
		if (num_pairs == 0) {
			out << "\n";
		}

		num_pairs++;
		out << std::string(indent + 1, '\t') << arg.name << ": " << to_string(arg.val, indent + 2) << "\n";
	};

	((void)(stringify_pair(named_vals)), ...);

	out << std::string(indent, '\t') << "}";

	return out.str();
}
