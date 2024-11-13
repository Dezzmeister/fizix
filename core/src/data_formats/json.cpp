#include <sstream>
#include "data_formats/json.h"
#include "data_formats/parsing.h"

namespace {
	json_value_or_descriptor parse_value(parsing::parser_state &state, json &json_out);
	bool parse_true(parsing::parser_state &state) {
		if (
			state.get() != 'r' ||
			state.get() != 'u' ||
			state.get() != 'e'
			) {
			throw json_parse_error("Expected 'true'\n", state.get_line_num(), state.get_col_num());
		}

		return true;
	}

	bool parse_false(parsing::parser_state &state) {
		if (
			state.get() != 'a' ||
			state.get() != 'l' ||
			state.get() != 's' ||
			state.get() != 'e'
			) {
			throw json_parse_error("Expected 'false'\n", state.get_line_num(), state.get_col_num());
		}

		return false;
	}

	std::nullptr_t parse_null(parsing::parser_state &state) {
		if (
			state.get() != 'u' ||
			state.get() != 'l' ||
			state.get() != 'l'
			) {
			throw json_parse_error("Expected 'null'\n", state.get_line_num(), state.get_col_num());
		}

		return nullptr;
	}

	bool parse_fraction(parsing::parser_state &state, std::wstringstream &out) {
		switch (state.peek()) {
			case L'.': {
				out << state.get();
				parsing::parse_dec_digits(state, out);
				return true;
			}
			default:
				return false;
		}
	}

	void parse_sign(parsing::parser_state &state, std::wstringstream &out) {
		switch (state.peek()) {
			case L'+':
			case L'-': {
				out << state.get();
				return;
			}
		}
	}

	bool parse_exponent(parsing::parser_state &state, std::wstringstream &out) {
		switch (state.peek()) {
			case L'E':
			case L'e': {
				out << state.get();
				parse_sign(state, out);
				parsing::parse_dec_digits(state, out);
				return true;
			}
			default:
				return false;
		}
	}

	void parse_integer(parsing::parser_state &state, std::wstringstream &out) {
		bool is_first_digit = true;

		if (state.peek() == L'-') {
			out << state.get();
		}

		while (! state.eof()) {
			switch (state.peek()) {
				case L'0': {
					out << state.get();

					if (is_first_digit) {
						return;
					} else {
						continue;
					}
				}
				case L'1':
				case L'2':
				case L'3':
				case L'4':
				case L'5':
				case L'6':
				case L'7':
				case L'8':
				case L'9': {
					is_first_digit = false;

					out << state.get();
					continue;
				}
				default:
					return;
			}
		}
	}

	json_value_or_descriptor parse_number(parsing::parser_state &state) {
		std::wstringstream out_str{};

		parse_integer(state, out_str);
		bool is_frac = parse_fraction(state, out_str);
		bool is_exp = parse_exponent(state, out_str);

		try {
			if (is_frac || is_exp) {
				return std::stod(out_str.str());
			} else {
				return std::stol(out_str.str());
			}
		} catch (std::invalid_argument _) {
			throw json_parse_error("Invalid number\n", state.get_line_num(), state.get_col_num());
		} catch (std::out_of_range _) {
			throw json_parse_error("Number out of range\n", state.get_line_num(), state.get_col_num());
		}
	}

	wchar_t parse_unicode_escape(parsing::parser_state &state) {
		wchar_t out = 0;

		for (int i = 3; i >= 0; i--) {
			int sh = i * 4;

			switch (state.get()) {
				case L'0':
					continue;
				case L'1':
				case L'2':
				case L'3':
				case L'4':
				case L'5':
				case L'6':
				case L'7':
				case L'8':
				case L'9': {
					out |= ((state.get_curr_char() - L'0') << sh);
					continue;
				}
				case L'a':
				case L'b':
				case L'c':
				case L'd':
				case L'e':
				case L'f': {
					out |= ((state.get_curr_char() - L'a' + 0xa) << sh);
					continue;
				}
				case L'A':
				case L'B':
				case L'C':
				case L'D':
				case L'E':
				case L'F': {
					out |= ((state.get_curr_char() - L'A' + 0xa) << sh);
					continue;
				}
				default:
					throw json_parse_error("Invalid Unicode escape sequence\n", state.get_line_num(), state.get_col_num());
			}
		}

		return out;
	}

	std::wstring parse_string(parsing::parser_state &state) {
		std::wstringstream out_str{};

		bool is_escape_seq = false;
		size_t start_line_num = state.get_line_num();
		size_t start_col_num = state.get_col_num();

		while (! state.eof()) {
			if (is_escape_seq) {
				switch (state.get()) {
					case L'"': {
						out_str << L'"';
						is_escape_seq = false;
						continue;
					}
					case L'\\': {
						out_str << L'\\';
						is_escape_seq = false;
						continue;
					}
					case L'/': {
						out_str << L'/';
						is_escape_seq = false;
						continue;
					}
					case L'b': {
						out_str << L'\b';
						is_escape_seq = false;
						continue;
					}
					case L'f': {
						out_str << L'\f';
						is_escape_seq = false;
						continue;
					}
					case L'n': {
						out_str << L'\n';
						is_escape_seq = false;
						continue;
					}
					case L'r': {
						out_str << L'\r';
						is_escape_seq = false;
						continue;
					}
					case L't': {
						out_str << L'\t';
						is_escape_seq = false;
						continue;
					}
					case L'u': {
						out_str << parse_unicode_escape(state);
						is_escape_seq = false;
						continue;
					}
					default:
						throw json_parse_error("Invalid escape sequence\n", state.get_line_num(), state.get_col_num());
				}
			}

			switch (state.get()) {
				case L'\\': {
					is_escape_seq = true;
					continue;
				}
				case L'"':
					return out_str.str();
				default: {
					wchar_t curr_char = state.get_curr_char();

					// This check filters out Unicode control characters, which are:
					// U+0000 - U+001f
					// U+007f
					// U+0080 - U+009f
					if (!(curr_char & ~0x1f) || !((curr_char ^ 0x80) & ~0x1f) || curr_char == 0x7f) {
						throw json_parse_error("Control character not allowed in string\n", state.get_line_num(), state.get_col_num());
					} else {
						out_str << state.get_curr_char();
					}
				}
			}
		}

		throw json_parse_error("Unterminated string\n", start_line_num, start_col_num);
	}

	json_array_descriptor parse_array(parsing::parser_state &state, json &json_out) {
		json_array out{};

		size_t start_line_num = state.get_line_num();
		size_t start_col_num = state.get_col_num();

		while (! state.eof()) {
			parse_whitespace(state);

			wchar_t next_char = state.peek();

			switch (next_char) {
				case L']': {
					state.get();

					return json_out.add(out);
				}
				case L',': {
					state.get();
					out.push_back(parse_value(state, json_out));
					continue;
				}
				default: {
					out.push_back(parse_value(state, json_out));
					continue;
				}
			}
		}

		throw json_parse_error("Unterminated array\n", start_line_num, start_col_num);
	}

	std::wstring parse_object_key(parsing::parser_state &state) {
		parse_whitespace(state);

		if (state.get() != L'"') {
			throw json_parse_error("Expected a quoted string\n", state.get_line_num(), state.get_col_num());
		}

		std::wstring key = parse_string(state);

		parse_whitespace(state);

		return key;
	}

	json_object_descriptor parse_object(parsing::parser_state &state, json &json_out) {
		json_object out{};

		size_t start_line_num = state.get_line_num();
		size_t start_col_num = state.get_col_num();

		if (state.peek() == L'}') {
			state.get();

			return json_out.add(out);
		}

		while (! state.eof()) {
			std::wstring key = parse_object_key(state);

			if (state.get() != L':') {
				throw json_parse_error("Expected ':'\n", state.get_line_num(), state.get_col_num());
			}

			json_value_or_descriptor value = parse_value(state, json_out);

			out[key] = value;

			parse_whitespace(state);

			switch (state.get()) {
				case L'}':
					return json_out.add(out);
				case L',':
					continue;
				default:
					throw json_parse_error("Unexpected char\n", state.get_line_num(), state.get_col_num());
			}
		}

		throw json_parse_error("Unexpected end of object\n", start_line_num, start_col_num);
	}

	json_value_or_descriptor parse_value(parsing::parser_state &state, json &json_out) {
		parse_whitespace(state);

		json_value_or_descriptor out{ nullptr };

		if (state.eof()) {
			throw json_parse_error("Unexpected EOF\n", state.get_line_num(), state.get_col_num());
		}

		switch (state.peek()) {
			case L'-':
			case L'0':
			case L'1':
			case L'2':
			case L'3':
			case L'4':
			case L'5':
			case L'6':
			case L'7':
			case L'8':
			case L'9':
			case L'.':
			case L'E':
			case L'e': {
				out = parse_number(state);
				break;
			}
			case L'"': {
				state.get();
				out = parse_string(state);
				break;
			}
			case L'[': {
				state.get();
				out = parse_array(state, json_out);
				break;
			}
			case L'{': {
				state.get();
				out = parse_object(state, json_out);
				break;
			}
			case 't': {
				state.get();
				out = parse_true(state);
				break;
			}
			case 'f': {
				state.get();
				out = parse_false(state);
				break;
			}
			case 'n': {
				state.get();
				out = parse_null(state);
				break;
			}
			default:
				throw new json_parse_error("Unexpected char\n", state.get_line_num(), state.get_col_num());
		}

		parse_whitespace(state);

		return out;
	}
}

bool operator==(const json_object_descriptor &a, const json_object_descriptor &b) {
	return a.obj_pos == b.obj_pos;
}

bool operator==(const json_array_descriptor &a, const json_array_descriptor &b) {
	return a.arr_pos == b.arr_pos;
}

std::optional<json_object *> json::get(json_object_descriptor desc) noexcept {
	if (desc.obj_pos >= objects.size()) {
		return std::nullopt;
	}

	return &objects[desc.obj_pos];
}

std::optional<json_array *> json::get(json_array_descriptor desc) noexcept {
	if (desc.arr_pos >= arrays.size()) {
		return std::nullopt;
	}

	return &arrays[desc.arr_pos];
}

json_value_or_descriptor json::get_root() const noexcept {
	return root;
}

json_object_descriptor json::add(json_object obj) {
	objects.push_back(obj);

	return {
		objects.size() - 1
	};
}

json_array_descriptor json::add(json_array arr) {
	arrays.push_back(arr);

	return {
		arrays.size() - 1
	};
}

size_t json::num_objects() const noexcept {
	return objects.size();
}

size_t json::num_arrays() const noexcept {
	return arrays.size();
}

json_parse_error::json_parse_error(const std::string &message, size_t _line_num, size_t _col_num) :
	std::runtime_error("Error parsing JSON (line " + std::to_string(_line_num) + ", col " + std::to_string(_col_num) + "): " + message),
	line_num(_line_num),
	col_num(_col_num)
{}

json parse_json(std::wistream &_in) {
	parsing::parser_state state(_in);
	json out{};

	out.root = parse_value(state, out);

	parse_whitespace(state);

	if (! state.eof()) {
		throw json_parse_error("Unexpected char\n", state.get_line_num(), state.get_col_num());
	}

	return out;
}