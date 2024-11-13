#include "data_formats/base64.h"

namespace {
	// Returns true if a character is a valid, non-padding base64 char
	bool is_base64_char(wchar_t c) {
		return (c >= L'A' && c <= L'Z') ||
			(c >= L'a' && c <= L'z') ||
			(c >= L'0' && c <= L'9') ||
			c == L'+' || c == L'/';
	}

	// Converts a base64 char into its numeric representation. The input must be
	// a valid, non-padding base64 char.
	uint8_t decode_char(wchar_t b64) {
		if (b64 == L'/') {
			return 0x3f;
		} else if (b64 == L'+') {
			return 0x3e;
		} else if (b64 <= L'9') {
			return (uint8_t)(b64 - L'0' + 0x34);
		} else if (b64 <= L'Z') {
			return (uint8_t)(b64 - L'A');
		} else {
			return (uint8_t)(b64 - L'a' + 0x1a);
		}
	}
}

base64_error::base64_error(const std::string &message, size_t _char_pos) :
	std::runtime_error(message),
	char_pos(_char_pos)
{}

std::vector<uint8_t> decode_base64(std::wstring &_in) {
	std::vector<uint8_t> out{};

	if (_in.empty() || _in[0] == L'=') {
		return out;
	}

	size_t padding = _in.size();
	while (padding && _in[--padding] == L'=');
	padding++;

	for (size_t i = 0; i < padding; i++) {
		if (! is_base64_char(_in[i]) && _in[i] != L'=') {
			throw base64_error("Invalid base64 at char " + std::to_string(i), i);
		}
	}

	size_t i = 0;

	for (; (i + 3) < padding; i += 4) {
		uint8_t b0 = decode_char(_in[i]);
		uint8_t b1 = decode_char(_in[i + 1]);
		uint8_t b2 = decode_char(_in[i + 2]);
		uint8_t b3 = decode_char(_in[i + 3]);

		uint8_t c0 = (b0 << 2) | (b1 >> 4);
		uint8_t c1 = ((b1 & 0xF) << 4) | (b2 >> 2);
		uint8_t c2 = ((b2 & 0x3) << 6) | b3;

		out.push_back(c0);
		out.push_back(c1);
		out.push_back(c2);
	}

	if (padding - i == 0) {
		return out;
	} else if (padding - i == 1) {
		uint8_t b0 = decode_char(_in[i]);

		out.push_back(b0 << 2);
	} else if (padding - i == 2) {
		uint8_t b0 = decode_char(_in[i]);
		uint8_t b1 = decode_char(_in[i + 1]);

		out.push_back((b0 << 2) | (b1 >> 4));
	} else if (padding - i == 3) {
		uint8_t b0 = decode_char(_in[i]);
		uint8_t b1 = decode_char(_in[i + 1]);
		uint8_t b2 = decode_char(_in[i + 2]);

		out.push_back((b0 << 2) | (b1 >> 4));
		out.push_back((b1 & 0xF) << 4 | (b2 >> 2));
	}

	return out;
}

std::vector<uint8_t> decode_base64(std::wistream &_in) {
	std::vector<uint8_t> out{};

	size_t i = 0;
	wchar_t char_buf[4];

	while (! _in.eof()) {
		char_buf[0] = _in.get();
		if (_in.eof() || char_buf[0] == L'=') {
			break;
		}
		if (! is_base64_char(char_buf[0])) {
			throw base64_error("Invalid base64 at char " + std::to_string(i), i);
		}
		i++;

		char_buf[1] = _in.get();
		if (_in.eof() || char_buf[1] == L'=') {
			break;
		}
		if (! is_base64_char(char_buf[0])) {
			throw base64_error("Invalid base64 at char " + std::to_string(i), i);
		}
		i++;

		char_buf[2] = _in.get();
		if (_in.eof() || char_buf[2] == L'=') {
			break;
		}
		if (! is_base64_char(char_buf[0])) {
			throw base64_error("Invalid base64 at char " + std::to_string(i), i);
		}
		i++;

		char_buf[3] = _in.get();
		if (_in.eof() || char_buf[3] == L'=') {
			break;
		}
		if (! is_base64_char(char_buf[0])) {
			throw base64_error("Invalid base64 at char " + std::to_string(i), i);
		}
		i++;

		uint8_t b0 = decode_char(char_buf[0]);
		uint8_t b1 = decode_char(char_buf[1]);
		uint8_t b2 = decode_char(char_buf[2]);
		uint8_t b3 = decode_char(char_buf[3]);

		uint8_t c0 = (b0 << 2) | (b1 >> 4);
		uint8_t c1 = ((b1 & 0xF) << 4) | (b2 >> 2);
		uint8_t c2 = ((b2 & 0x3) << 6) | b3;

		out.push_back(c0);
		out.push_back(c1);
		out.push_back(c2);
	}

	while (! _in.eof() && _in.peek() == L'=') {
		_in.get();
	}

	if ((i & 0x3) == 0) {
		return out;
	} else if ((i & 0x3) == 1) {
		uint8_t b0 = decode_char(char_buf[0]);

		out.push_back(b0 << 2);
	} else if ((i & 0x3) == 2) {
		uint8_t b0 = decode_char(char_buf[0]);
		uint8_t b1 = decode_char(char_buf[1]);

		out.push_back((b0 << 2) | (b1 >> 4));
	} else if ((i & 0x3) == 3) {
		uint8_t b0 = decode_char(char_buf[0]);
		uint8_t b1 = decode_char(char_buf[1]);
		uint8_t b2 = decode_char(char_buf[2]);

		out.push_back((b0 << 2) | (b1 >> 4));
		out.push_back((b1 & 0xF) << 4 | (b2 >> 2));
	}

	return out;
}