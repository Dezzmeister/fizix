#include "data_formats/parsing.h"

namespace parsing {
	parser_state::parser_state(std::wistream &_in) :
		in(_in)
	{}

	wchar_t parser_state::get() {
		wchar_t next = (curr_char = in.get());

		if (next == L'\n') {
			line_num++;
			col_num = 0L;
		} else {
			col_num++;
		}

		if (next != WEOF) {
			total_chars++;
		}

		return next;
	}

	wchar_t parser_state::peek() const {
		return in.peek();
	}

	bool parser_state::eof() const {
		return in.eof();
	}

	wchar_t parser_state::get_curr_char() const {
		return curr_char;
	}

	size_t parser_state::get_line_num() const {
		return line_num;
	}

	size_t parser_state::get_col_num() const {
		return col_num;
	}

	size_t parser_state::get_total_chars() const {
		return total_chars;
	}

	std::wstringstream parser_state::drain() {
		std::wstringstream out{};

		while (peek() != WEOF) {
			out << get();
		}

		return out;
	}

	size_t parse_whitespace(parser_state &state) {
		size_t start = state.get_total_chars();

		while (! state.eof()) {
			wchar_t next_char = state.peek();

			switch (next_char) {
				case ' ':
				case '\r':
				case '\t':
				case '\n': {
					state.get();
					continue;
				}
				default:
					return state.get_total_chars() - start;
			}
		}

		return state.get_total_chars() - start;
	}

	size_t parse_dec_digits(parser_state &state, std::wstringstream &out) {
		size_t start = state.get_total_chars();

		while (! state.eof()) {
			switch (state.peek()) {
				case L'0':
				case L'1':
				case L'2':
				case L'3':
				case L'4':
				case L'5':
				case L'6':
				case L'7':
				case L'8':
				case L'9': {
					out << state.get();
					continue;
				}
				default:
					return state.get_total_chars() - start;
			}
		}

		return state.get_total_chars() - start;
	}

	int8_t parse_digit(parser_state &state) {
		wchar_t curr_char = state.peek();

		if (curr_char >= L'0' && curr_char <= L'9') {
			return (int8_t)(state.get() - L'0');
		}

		return -1;
	}

	int8_t parse_hexdig(parser_state &state) {
		int8_t digit = parse_digit(state);

		if (digit != -1) {
			return digit & 0xFF;
		}

		switch (state.peek()) {
			case L'A':
			case L'B':
			case L'C':
			case L'D':
			case L'E':
			case L'F':
				return (int8_t)(state.get() - L'A' + 0xa);
			case L'a':
			case L'b':
			case L'c':
			case L'd':
			case L'e':
			case L'f':
				return (int8_t)(state.get() - L'a' + 0xa);
			default:
				return -1;
		}
	}

	bool parse_alpha(parser_state &state, std::wstringstream &out) {
		wchar_t curr_char = state.peek();

		if (
			(curr_char >= L'A' && curr_char <= L'Z') ||
			(curr_char >= L'a' && curr_char <= L'z')
		) {
			out << state.get();
			return true;
		}

		return false;
	}

	bool parse_digit(parser_state &state, std::wstringstream &out) {
		wchar_t curr_char = state.peek();

		if (curr_char >= L'0' && curr_char <= L'9') {
			out << state.get();
			return true;
		}

		return false;
	}

	bool parse_hexdig(parser_state &state, std::wstringstream &out) {
		if (parse_digit(state, out)) {
			return true;
		}

		switch (state.peek()) {
			case L'A':
			case L'B':
			case L'C':
			case L'D':
			case L'E':
			case L'F':
			case L'a':
			case L'b':
			case L'c':
			case L'd':
			case L'e':
			case L'f': {
				out << state.get();
				return true;
			}
			default:
				return false;
		}
	}

	bool parse_one_char(parser_state &state, wchar_t expected, std::wstringstream &out) {
		if (state.peek() == expected) {
			out << state.get();
			return true;
		}

		return false;
	}

	bool parse_string(parser_state &state, const std::wstring &expected, std::wstringstream &out) {
		for (wchar_t c : expected) {
			if (! parse_one_char(state, c, out)) {
				return false;
			}
		}

		return true;
	}

	size_t parse_until(parser_state &state, wchar_t sentinel, std::wstringstream &out) {
		size_t start = state.get_total_chars();

		while (! state.eof() && state.peek() != sentinel) {
			out << state.get();
		}

		return state.get_total_chars() - start;
	}

	bool parse_char_in_range(parser_state &state, wchar_t start, wchar_t end, std::wstringstream &out) {
		wchar_t curr_char = state.peek();

		if (start <= curr_char && curr_char <= end) {
			out << state.get();
			return true;
		}

		return false;
	}

	bool is_digit(wchar_t c) {
		return (L'0' <= c && c <= L'9');
	}

	bool is_hexdig(wchar_t c) {
		return (L'0' <= c && c <= L'9') ||
			(L'A' <= c && c <= L'F') ||
			(L'a' <= c && c <= L'f');
	}
}