#pragma once
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

std::vector<uint8_t> decode_base64(std::wstring &_in);
std::vector<uint8_t> decode_base64(std::wistream &_in);

class base64_error : public std::runtime_error {
public:
	const size_t char_pos;

	base64_error(const std::string &message, size_t _char_pos);
};
