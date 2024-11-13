#include "data_formats/uri.h"
#include "test.h"

using namespace test;

namespace {
	uri parse_uri_helper(std::wstring str) {
		std::wstringstream wss(str);

		return parse_uri(wss);
	}
}

void setup_uri_tests() {
	describe("URI", []() {
		describe("Scheme part", []() {
			it("Parses an http scheme with an authority", []() {
				uri result = parse_uri_helper(L"http://google.com/the/rest/of/the/uri");
				
				expect_cond(result.scheme == L"http");
				expect_cond(result.path_and_rest == L"/the/rest/of/the/uri");
			});

			it("Parses a file scheme with an absolute path", []() {
				uri result = parse_uri_helper(L"file:/absolute/path/to/a/file");
				
				expect_cond(result.scheme == L"file");
				expect_cond(result.path_and_rest == L"/absolute/path/to/a/file");
			});

			it("Parses an unknown scheme with an empty path", []() {
				uri result = parse_uri_helper(L"unknown-scheme:");

				expect_cond(result.scheme == L"unknown-scheme");
				expect_cond(result.path_and_rest == L"");
			});

			it("Parses an unknown scheme with unusual albeit valid characters", []() {
				uri result = parse_uri_helper(L"unknown-sch3m3-with+.-+-+weird-ch4r4ct3r5://127.0.0.1");

				expect_cond(result.scheme == L"unknown-sch3m3-with+.-+-+weird-ch4r4ct3r5");
				expect_cond(result.path_and_rest == L"");
			});

			it("Fails to parse a scheme that contains invalid characters", []() {
				try {
					uri result = parse_uri_helper(L"scheme*with-@%-invalid-chars://google.com");
					fail_msg("Expected parser to throw");
				} catch (uri_error err) {
					expect_cond(err.char_pos == 6);
				}
			});

			it("Fails to parse a scheme that starts with a non-ALPHA", []() {
				try {
					uri result = parse_uri_helper(L"9scheme://google.com");
					fail_msg("Expected parser to throw");
				} catch (uri_error err) {
					expect_cond(err.char_pos == 0);
				}
			});
		});

		describe("Authority part", []() {
			it("Parses a reg-name host without a userinfo or a port", []() {
				uri result = parse_uri_helper(L"https://google.com?query=string");

				expect_msg("userinfo", result.authority.userinfo == std::nullopt);
				expect_msg("host type", std::holds_alternative<std::wstring>(result.authority.host));
				expect_msg("host", std::get<std::wstring>(result.authority.host) == L"google.com");
				expect_msg("port", result.authority.port == std::nullopt);
				expect_msg("path_and_rest", result.path_and_rest == L"?query=string");
			});

			it("Parses an IPv4 host without a userinfo or a port", []() {
				uri result = parse_uri_helper(L"https://127.0.0.1/path");

				expect_msg("userinfo", result.authority.userinfo == std::nullopt);
				expect_msg("host type", std::holds_alternative<ipv4_addr>(result.authority.host));
				expect_msg("host", std::get<ipv4_addr>(result.authority.host) == ipv4_addr(127, 0, 0, 1));
				expect_msg("port", result.authority.port == std::nullopt);
				expect_msg("path_and_rest", result.path_and_rest == L"/path");
			});

			it("Parses an IPv6 host without a userinfo or a port", []() {
				uri result = parse_uri_helper(L"https://[feeb:beef:1234::9]/path");

				expect_msg("userinfo", result.authority.userinfo == std::nullopt);
				expect_msg("host type", std::holds_alternative<ipv6_addr>(result.authority.host));
				expect_msg("host", std::get<ipv6_addr>(result.authority.host) ==
					ipv6_addr(0xfeeb, 0xbeef, 0x1234, 0, 0, 0, 0, 0x9));
				expect_msg("port", result.authority.port == std::nullopt);
				expect_msg("path_and_rest", result.path_and_rest == L"/path");
			});

			it("Parses a reg-name host with a port but no userinfo", []() {
				uri result = parse_uri_helper(L"http://localhost:1234/path");

				expect_msg("userinfo", result.authority.userinfo == std::nullopt);
				expect_msg("host type", std::holds_alternative<std::wstring>(result.authority.host));
				expect_msg("host", std::get<std::wstring>(result.authority.host) == L"localhost");
				expect_msg("port", result.authority.port == 1234);
				expect_msg("path_and_rest", result.path_and_rest == L"/path");
			});

			it("Parses an IPv4 host with a port but no userinfo", []() {
				uri result = parse_uri_helper(L"https://127.0.0.1:8000/path");

				expect_msg("userinfo", result.authority.userinfo == std::nullopt);
				expect_msg("host type", std::holds_alternative<ipv4_addr>(result.authority.host));
				expect_msg("host", std::get<ipv4_addr>(result.authority.host) == ipv4_addr(127, 0, 0, 1));
				expect_msg("port", result.authority.port == 8000);
				expect_msg("path_and_rest", result.path_and_rest == L"/path");
			});

			it("Parses an IPv6 host with a port but no userinfo", []() {
				uri result = parse_uri_helper(L"https://[feeb:beef:1234::9]:65535/path?query");

				expect_msg("userinfo", result.authority.userinfo == std::nullopt);
				expect_msg("host type", std::holds_alternative<ipv6_addr>(result.authority.host));
				expect_msg("host", std::get<ipv6_addr>(result.authority.host) ==
					ipv6_addr(0xfeeb, 0xbeef, 0x1234, 0, 0, 0, 0, 0x9));
				expect_msg("port", result.authority.port == 65535);
				expect_msg("path_and_rest", result.path_and_rest == L"/path?query");
			});

			it("Parses a reg-name host with a userinfo but no port", []() {
				uri result = parse_uri_helper(L"https://username@google.com?query=string");

				expect_msg("userinfo", result.authority.userinfo == L"username");
				expect_msg("host type", std::holds_alternative<std::wstring>(result.authority.host));
				expect_msg("host", std::get<std::wstring>(result.authority.host) == L"google.com");
				expect_msg("port", result.authority.port == std::nullopt);
				expect_msg("path_and_rest", result.path_and_rest == L"?query=string");
			});

			it("Parses an IPv4 host with a userinfo but no port", []() {
				uri result = parse_uri_helper(L"https://username:password@127.0.0.1/path");

				expect_msg("userinfo", result.authority.userinfo == L"username:password");
				expect_msg("host type", std::holds_alternative<ipv4_addr>(result.authority.host));
				expect_msg("host", std::get<ipv4_addr>(result.authority.host) == ipv4_addr(127, 0, 0, 1));
				expect_msg("port", result.authority.port == std::nullopt);
				expect_msg("path_and_rest", result.path_and_rest == L"/path");
			});

			it("Parses an IPv6 host with a userinfo but no port", []() {
				uri result = parse_uri_helper(L"https://username:port@[feeb:beef:1234::9]/path");

				expect_msg("userinfo", result.authority.userinfo == L"username:port");
				expect_msg("host type", std::holds_alternative<ipv6_addr>(result.authority.host));
				expect_msg("host", std::get<ipv6_addr>(result.authority.host) ==
					ipv6_addr(0xfeeb, 0xbeef, 0x1234, 0, 0, 0, 0, 0x9));
				expect_msg("port", result.authority.port == std::nullopt);
				expect_msg("path_and_rest", result.path_and_rest == L"/path");
			});

			it("Parses a reg-name host with a port and a userinfo", []() {
				uri result = parse_uri_helper(L"http://userinfo@localhost:1234/path");

				expect_msg("userinfo", result.authority.userinfo == L"userinfo");
				expect_msg("host type", std::holds_alternative<std::wstring>(result.authority.host));
				expect_msg("host", std::get<std::wstring>(result.authority.host) == L"localhost");
				expect_msg("port", result.authority.port == 1234);
				expect_msg("path_and_rest", result.path_and_rest == L"/path");
			});

			it("Parses an IPv4 host with a port and a userinfo", []() {
				uri result = parse_uri_helper(L"https://1234:5678@127.0.0.1:8000/path");

				expect_msg("userinfo", result.authority.userinfo == L"1234:5678");
				expect_msg("host type", std::holds_alternative<ipv4_addr>(result.authority.host));
				expect_msg("host", std::get<ipv4_addr>(result.authority.host) == ipv4_addr(127, 0, 0, 1));
				expect_msg("port", result.authority.port == 8000);
				expect_msg("path_and_rest", result.path_and_rest == L"/path");
			});

			it("Parses an IPv6 host with a port and a userinfo", []() {
				uri result = parse_uri_helper(L"https://u:p:p:p@[feeb:beef:1234::9]:65535/path?query");

				expect_msg("userinfo", result.authority.userinfo == L"u:p:p:p");
				expect_msg("host type", std::holds_alternative<ipv6_addr>(result.authority.host));
				expect_msg("host", std::get<ipv6_addr>(result.authority.host) ==
					ipv6_addr(0xfeeb, 0xbeef, 0x1234, 0, 0, 0, 0, 0x9));
				expect_msg("port", result.authority.port == 65535);
				expect_msg("path_and_rest", result.path_and_rest == L"/path?query");
			});

			it("Parses a reg-name that starts with an IPv4 address", []() {
				uri result = parse_uri_helper(L"http://127.0.0.1reg-name.co.uk/path");

				expect_msg("userinfo", result.authority.userinfo == std::nullopt);
				expect_msg("host type", std::holds_alternative<std::wstring>(result.authority.host));
				expect_msg("host", std::get<std::wstring>(result.authority.host) == L"127.0.0.1reg-name.co.uk");
				expect_msg("port", result.authority.port == std::nullopt);
				expect_msg("path_and_rest", result.path_and_rest == L"/path");
			});

			it("Fails to parse a URI with a colon at the end and no port", []() {
				try {
					uri result = parse_uri_helper(L"http://10.0.0.1:/path");
					fail_msg("Expected parser to throw");
				} catch (uri_error err) {
					expect_cond(err.char_pos == 16);
				}
			});

			it("Fails to parse an invalid reg-name", []() {
				try {
					uri result = parse_uri_helper(L"http://invalid[char/path?query");
					fail_msg("Expected parser to throw");
				} catch (uri_error err) {
					expect_cond(err.char_pos == 14);
				}
			});
		});
	});
}