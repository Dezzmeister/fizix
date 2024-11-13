#pragma once
#include <iostream>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <variant>
#include <vector>

class json;
json parse_json(std::wistream &_in);

struct json_object_descriptor {
	size_t obj_pos{ 0 };

	friend bool operator==(const json_object_descriptor &a, const json_object_descriptor &b);
};

struct json_array_descriptor {
	size_t arr_pos{ 0 };

	friend bool operator==(const json_array_descriptor &a, const json_array_descriptor &b);
};

using json_value_or_descriptor = std::variant<json_object_descriptor, json_array_descriptor, std::wstring, long, double, bool, std::nullptr_t>;
using json_object = std::unordered_map<std::wstring, json_value_or_descriptor>;
using json_array = std::vector<json_value_or_descriptor>;

// A self-contained JSON structure. An instance of this class represents a valid JSON
// value, and it owns all of its sub-objects and sub-arrays. A JSON element is either
// a value or a descriptor. Strings and primitives are directly accessible as values,
// and arrays & objects are represented by "descriptors" that can be used to get the
// actual array/object.
class json {
public:
	// Fetches a JSON object given an object descriptor. Returns std::nullopt if the 
	// object descriptor does not point to a valid object.
	std::optional<json_object *> get(json_object_descriptor desc) noexcept;
	// Fetches a JSON array given an array descriptor. Returns std::nullopt if the array
	// descriptor does not point to a valid array;
	std::optional<json_array *> get(json_array_descriptor desc) noexcept;
	json_value_or_descriptor get_root() const noexcept;

	// Adds a JSON object to the list of objects owned by this JSON structure. An object
	// descriptor is created and returned to the caller. Until the caller puts the descriptor
	// somewhere in the JSON structure, the object is not referenced and is not part of the
	// JSON structure.
	json_object_descriptor add(json_object obj);
	// Adds a JSON array to the list of arrays owned by this JSON structure. An array descriptor
	// is created and returned to the caller. Until the caller puts the descriptor somewhere in
	// the JSON structure, the array is not referenced and is not part of the JSON structure.
	json_array_descriptor add(json_array arr);

	size_t num_objects() const noexcept;
	size_t num_arrays() const noexcept;

private:
	std::vector<json_object> objects{};
	std::vector<json_array> arrays{};
	json_value_or_descriptor root{ nullptr };

	friend json parse_json(std::wistream &_in);
};

class json_parse_error : public std::runtime_error {
public:
	const size_t line_num;
	const size_t col_num;

	json_parse_error(const std::string &message, size_t _line_num, size_t _col_num);
};
