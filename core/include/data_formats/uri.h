#pragma once
#include <concepts>
#include <iostream>
#include <string>
#include "ipaddr.h"

// A host, almost exactly as defined in RFC 3986, with one exception: IPvFuture
// is not supported.
using uri_host = std::variant<ipv4_addr, ipv6_addr, std::wstring>;

struct uri_authority {
	std::optional<std::wstring> userinfo{};
	uri_host host{};
	std::optional<uint16_t> port{};
};

struct uri {
	std::wstring scheme{};
	uri_authority authority{};
	// The rest of the URI, starting with the path. We don't actually need
	// to parse this for our purposes.
	std::wstring path_and_rest{};
};

class uri_error : public std::runtime_error {
public:
	const size_t char_pos;

	uri_error(const std::string &message, size_t _char_pos);
};

uri parse_uri(std::wistream &_in);
