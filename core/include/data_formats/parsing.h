#pragma once
#include <iostream>
#include <sstream>

namespace parsing {
	// A utility class for LL(1) parsing. A parser using this state has a 1-char
	// lookahead and is unable to put chars back into the stream.
	class parser_state {
	public:
		parser_state(std::wistream &_in);

		// Removes the next char from the stream and returns it. If the char is newline,
		// the line num will be incremented and the col num reset. Otherwise, the col num
		// will be incremented. If the end of the stream has been reached, returns WEOF.
		wchar_t get();
		// Peeks the next char in the stream without removing it.
		wchar_t peek() const;
		// Returns the last char that was removed from the stream. This may be WEOF if the
		// end of the stream has been reached.
		wchar_t get_curr_char() const;

		bool eof() const;
		size_t get_line_num() const;
		size_t get_col_num() const;
		size_t get_total_chars() const;

		std::wstringstream drain();

	private:
		std::wistream &in;
		wchar_t curr_char{ 0 };
		size_t line_num{ 1L };
		size_t col_num{ 0L };
		size_t total_chars{ 0L };
	};

	// Greedily parses whitespace (tab, newline, and carriage return).
	// Returns the number of chars parsed.
	size_t parse_whitespace(parser_state &state);

	// Parses decimal digits and writes them to an output stream. Returns the
	// number of chars parsed.
	size_t parse_dec_digits(parser_state &state, std::wstringstream &out);

	// Tries to parse an "alpha" as defined by RFC2234 ("A-Z / a-z").
	// Returns the alpha if successful; otherwise returns -1.
	int8_t parse_alpha(parser_state &state);

	// Tries to parse a single digit as defined by RFC2234 ("0-9").
	// Returns the numeric representation of the digit if successful;
	// otherwise returns -1.
	int8_t parse_digit(parser_state &state);

	// Tries to parse a single hex digit as defined by RFC2234.
	// Returns the numeric representation of the digit if successful;
	// otherwise returns -1.
	int8_t parse_hexdig(parser_state &state);

	// Tries to parse an "alpha" as defined by RFC2234. Writes the character to
	// the output stream and returns true if successful. Returns false if unsuccessful.
	bool parse_alpha(parser_state &state, std::wstringstream &out);

	// Tries to parse a single digit as defined by RFC2234. If successful,
	// writes the digit to the output stream (in its ASCII representation)
	// and returns true. Returns false if unsuccessful.
	bool parse_digit(parser_state &state, std::wstringstream &out);

	// Tries to parse a single digit as defined by RFC2234. If successful,
	// writes the digit to the output stream (in its ASCII representation)
	// and returns true. Returns false if unsuccessful.
	bool parse_hexdig(parser_state &state, std::wstringstream &out);

	// Tries to parse a single occurrence of the expected char. If successful, writes the
	// char to the output stream and returns true. Returns false if unsuccessful.
	bool parse_one_char(parser_state &state, wchar_t expected, std::wstringstream &out);

	// Parses a character in the range [start, end].
	bool parse_char_in_range(parser_state &state, wchar_t start, wchar_t end, std::wstringstream &out);

	bool is_digit(wchar_t c);
	bool is_hexdig(wchar_t c);
}
