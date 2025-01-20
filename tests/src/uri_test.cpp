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
				
				expect(result.scheme).to_be(L"http");
				expect(result.path_and_rest).to_be(L"/the/rest/of/the/uri");
			});

			it("Parses a file scheme with an absolute path", []() {
				uri result = parse_uri_helper(L"file:/absolute/path/to/a/file");
				
				expect(result.scheme).to_be(L"file");
				expect(result.path_and_rest).to_be(L"/absolute/path/to/a/file");
			});

			it("Parses an unknown scheme with an empty path", []() {
				uri result = parse_uri_helper(L"unknown-scheme:");

				expect(result.scheme).to_be(L"unknown-scheme");
				expect(result.path_and_rest).to_be(L"");
			});

			it("Parses an unknown scheme with unusual albeit valid characters", []() {
				uri result = parse_uri_helper(L"unknown-sch3m3-with+.-+-+weird-ch4r4ct3r5://127.0.0.1");

				expect(result.scheme).to_be(L"unknown-sch3m3-with+.-+-+weird-ch4r4ct3r5");
				expect(result.path_and_rest).to_be(L"");
			});

			it("Fails to parse a scheme that contains invalid characters", []() {
				try {
					uri result = parse_uri_helper(L"scheme*with-@%-invalid-chars://google.com");
					fail("Expected parser to throw");
				} catch (uri_error err) {
					expect(err.char_pos).to_be(6);
				}
			});

			it("Fails to parse a scheme that starts with a non-ALPHA", []() {
				try {
					uri result = parse_uri_helper(L"9scheme://google.com");
					fail("Expected parser to throw");
				} catch (uri_error err) {
					expect(err.char_pos).to_be(0);
				}
			});
		});

		describe("Authority part", []() {
			it("Parses a reg-name host without a userinfo or a port", []() {
				uri result = parse_uri_helper(L"https://google.com?query=string");

				expect(result.authority.userinfo).to_be_empty();
				expect(result.authority.host).to_be(L"google.com");
				expect(result.authority.port).to_be_empty();
				expect(result.path_and_rest).to_be(L"?query=string");
			});

			it("Parses an IPv4 host without a userinfo or a port", []() {
				uri result = parse_uri_helper(L"https://127.0.0.1/path");

				expect(result.authority.userinfo).to_be_empty();
				expect(result.authority.host).to_be(ipv4_addr(127, 0, 0, 1));
				expect(result.authority.port).to_be_empty();
				expect(result.path_and_rest).to_be(L"/path");
			});

			it("Parses an IPv6 host without a userinfo or a port", []() {
				uri result = parse_uri_helper(L"https://[feeb:beef:1234::9]/path");

				expect(result.authority.userinfo).to_be_empty();
				expect(result.authority.host).to_be(ipv6_addr(0xfeeb, 0xbeef, 0x1234, 0, 0, 0, 0, 0x9));
				expect(result.authority.port).to_be_empty();
				expect(result.path_and_rest).to_be(L"/path");
			});

			it("Parses a reg-name host with a port but no userinfo", []() {
				uri result = parse_uri_helper(L"http://localhost:1234/path");

				expect(result.authority.userinfo).to_be_empty();
				expect(result.authority.host).to_be(L"localhost");
				expect(result.authority.port).to_have_value(1234);
				expect(result.path_and_rest).to_be(L"/path");
			});

			it("Parses an IPv4 host with a port but no userinfo", []() {
				uri result = parse_uri_helper(L"https://127.0.0.1:8000/path");

				expect(result.authority.userinfo).to_be_empty();
				expect(result.authority.host).to_be(ipv4_addr(127, 0, 0, 1));
				expect(result.authority.port).to_have_value(8000);
				expect(result.path_and_rest).to_be(L"/path");
			});

			it("Parses an IPv6 host with a port but no userinfo", []() {
				uri result = parse_uri_helper(L"https://[feeb:beef:1234::9]:65535/path?query");

				expect(result.authority.userinfo).to_be_empty();
				expect(result.authority.host).to_be(ipv6_addr(0xfeeb, 0xbeef, 0x1234, 0, 0, 0, 0, 0x9));
				expect(result.authority.port).to_have_value(65535);
				expect(result.path_and_rest).to_be(L"/path?query");
			});

			it("Parses a reg-name host with a userinfo but no port", []() {
				uri result = parse_uri_helper(L"https://username@google.com?query=string");

				expect(result.authority.userinfo).to_have_value(L"username");
				expect(result.authority.host).to_be(L"google.com");
				expect(result.authority.port).to_be_empty();
				expect(result.path_and_rest).to_be(L"?query=string");
			});

			it("Parses an IPv4 host with a userinfo but no port", []() {
				uri result = parse_uri_helper(L"https://username:password@127.0.0.1/path");

				expect(result.authority.userinfo).to_have_value(L"username:password");
				expect(result.authority.host).to_be(ipv4_addr(127, 0, 0, 1));
				expect(result.authority.port).to_be_empty();
				expect(result.path_and_rest).to_be(L"/path");
			});

			it("Parses an IPv6 host with a userinfo but no port", []() {
				uri result = parse_uri_helper(L"https://username:password@[feeb:beef:1234::9]/path");

				expect(result.authority.userinfo).to_have_value(L"username:password");
				expect(result.authority.host).to_be(ipv6_addr(0xfeeb, 0xbeef, 0x1234, 0, 0, 0, 0, 0x9));
				expect(result.authority.port).to_be_empty();
				expect(result.path_and_rest).to_be(L"/path");
			});

			it("Parses a reg-name host with a port and a userinfo", []() {
				uri result = parse_uri_helper(L"http://userinfo@localhost:1234/path");

				expect(result.authority.userinfo).to_have_value(L"userinfo");
				expect(result.authority.host).to_be(L"localhost");
				expect(result.authority.port).to_have_value(1234);
				expect(result.path_and_rest).to_be(L"/path");
			});

			it("Parses an IPv4 host with a port and a userinfo", []() {
				uri result = parse_uri_helper(L"https://1234:5678@127.0.0.1:8000/path");

				expect(result.authority.userinfo).to_have_value(L"1234:5678");
				expect(result.authority.host).to_be(ipv4_addr(127, 0, 0, 1));
				expect(result.authority.port).to_have_value(8000);
				expect(result.path_and_rest).to_be(L"/path");
			});

			it("Parses an IPv6 host with a port and a userinfo", []() {
				uri result = parse_uri_helper(L"https://u:p:p:p@[feeb:beef:1234::9]:65535/path?query");

				expect(result.authority.userinfo).to_have_value(L"u:p:p:p");
				expect(result.authority.host).to_be(ipv6_addr(0xfeeb, 0xbeef, 0x1234, 0, 0, 0, 0, 0x9));
				expect(result.authority.port).to_have_value(65535);
				expect(result.path_and_rest).to_be(L"/path?query");
			});

			it("Parses a reg-name that starts with an IPv4 address", []() {
				uri result = parse_uri_helper(L"http://127.0.0.1reg-name.co.uk/path");

				expect(result.authority.userinfo).to_be_empty();
				expect(result.authority.host).to_be(L"127.0.0.1reg-name.co.uk");
				expect(result.authority.port).to_be_empty();
				expect(result.path_and_rest).to_be(L"/path");
			});

			it("Fails to parse a URI with a colon at the end and no port", []() {
				try {
					uri result = parse_uri_helper(L"http://10.0.0.1:/path");
					fail("Expected parser to throw");
				} catch (uri_error err) {
					expect(err.char_pos).to_be(16);
				}
			});

			it("Fails to parse an invalid reg-name", []() {
				try {
					uri result = parse_uri_helper(L"http://invalid[char/path?query");
					fail("Expected parser to throw");
				} catch (uri_error err) {
					expect(err.char_pos).to_be(14);
				}
			});
		});
	});
}