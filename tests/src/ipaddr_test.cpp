#include "data_formats/ipaddr.h"
#include "traits.h"
#include "test.h"

using namespace test;

namespace {
	template <typename IPAddr>
	struct ip_parse_out {
		std::optional<IPAddr> ip{};
		std::wstring out{};
	};

	ip_parse_out<ipv4_addr> parse_ipv4_helper(std::wstring in) {
		std::wstringstream in_stream(in);
		parsing::parser_state state(in_stream);
		std::wstringstream out{};
		std::optional<ipv4_addr> result = parse_ipv4(state, out);

		return {
			result,
			out.str()
		};
	}

	ip_parse_out<ipv6_addr> parse_ipv6_helper(std::wstring in) {
		std::wstringstream in_stream(in);
		parsing::parser_state state(in_stream);
		std::wstringstream out{};
		std::optional<ipv6_addr> result = parse_ipv6(state, out);

		return {
			result,
			out.str()
		};
	}
}

void setup_ipaddr_tests() {
	describe("IPv4", []() {
		it("Parses an IP with four octets all set to zero", []() {
			ip_parse_out<ipv4_addr> result = parse_ipv4_helper(L"0.0.0.0");

			expect(result.ip).to_have_value(ipv4_addr(0, 0, 0, 0));
			expect(result.out).to_be(L"0.0.0.0");
		});

		it("Parses an IP with four octets all set to 255", []() {
			ip_parse_out<ipv4_addr> result = parse_ipv4_helper(L"255.255.255.255");

			expect(result.ip).to_have_value(ipv4_addr(255, 255, 255, 255));
			expect(result.out).to_be(L"255.255.255.255");
		});

		it("Parses an IP with four octets between 0 and 255", []() {
			ip_parse_out<ipv4_addr> result = parse_ipv4_helper(L"1.23.109.45");

			expect(result.ip).to_have_value(ipv4_addr(1, 23, 109, 45));
			expect(result.out).to_be(L"1.23.109.45");
		});

		it("Fails to parse an IP with three octets", []() {
			ip_parse_out<ipv4_addr> result = parse_ipv4_helper(L"1.23.45");

			expect(result.ip).to_be_empty();
			expect(result.out).to_be(L"1.23.45");
		});

		it("Fails to parse an IP with two octets", []() {
			ip_parse_out<ipv4_addr> result = parse_ipv4_helper(L"23.45");

			expect(result.ip).to_be_empty();
			expect(result.out).to_be(L"23.45");
		});

		it("Fails to parse an IP with one octet", []() {
			ip_parse_out<ipv4_addr> result = parse_ipv4_helper(L"45");

			expect(result.ip).to_be_empty();
			expect(result.out).to_be(L"45");
		});

		it("Fails to parse an empty string", []() {
			ip_parse_out<ipv4_addr> result = parse_ipv4_helper(L"");

			expect(result.ip).to_be_empty();
			expect(result.out).to_be(L"");
		});

		it("Fails to parse an IP with one octet > 255", []() {
			ip_parse_out<ipv4_addr> result = parse_ipv4_helper(L"1.2.3.256");

			expect(result.ip).to_be_empty();
			expect(result.out).to_be(L"1.2.3.256");
		});

		it("Fails to parse an IP with an invalid character", []() {
			ip_parse_out<ipv4_addr> result = parse_ipv4_helper(L"192.168.x.100");

			expect(result.ip).to_be_empty();
			expect(result.out).to_be(L"192.168.");
		});

		it("Fails to parse an IP with a multi-digit octet with a leading zero", []() {
			ip_parse_out<ipv4_addr> result = parse_ipv4_helper(L"192.168.01.9");

			expect(result.ip).to_be_empty();
			expect(result.out).to_be(L"192.168.0");
		});
	});

	describe("IPv6", []() {
		it("Parses an IP with no pieces elided and two h16s at the end", []() {
			ip_parse_out<ipv6_addr> result = parse_ipv6_helper(L"1234:0191:f:1:2929:abcd:8888:dd");

			expect(result.ip).to_have_value(ipv6_addr(0x1234, 0x0191, 0xf, 0x1, 0x2929, 0xabcd, 0x8888, 0xdd));
			expect(result.out).to_be(L"1234:0191:f:1:2929:abcd:8888:dd");
		});

		it("Parses an IP with one piece elided and two h16s at the end", []() {
			ip_parse_out<ipv6_addr> result = parse_ipv6_helper(L"1234:0191::1:2929:abcd:8888:dd");

			expect(result.ip).to_have_value(ipv6_addr(0x1234, 0x0191, 0, 0x1, 0x2929, 0xabcd, 0x8888, 0xdd));
			expect(result.out).to_be(L"1234:0191::1:2929:abcd:8888:dd");
		});

		it("Parses an IP with two pieces elided and two h16s at the end", []() {
			ip_parse_out<ipv6_addr> result = parse_ipv6_helper(L"1234::1:2929:abcd:8888:dd");

			expect(result.ip).to_have_value(ipv6_addr(0x1234, 0, 0, 0x1, 0x2929, 0xabcd, 0x8888, 0xdd));
			expect(result.out).to_be(L"1234::1:2929:abcd:8888:dd");
		});

		it("Parses an IP with three pieces elided and two h16s at the end", []() {
			ip_parse_out<ipv6_addr> result = parse_ipv6_helper(L"1234::2929:abcd:8888:dd");

			expect(result.ip).to_have_value(ipv6_addr(0x1234, 0, 0, 0, 0x2929, 0xabcd, 0x8888, 0xdd));
			expect(result.out).to_be(L"1234::2929:abcd:8888:dd");
		});

		it("Parses an IP with four pieces elided and two h16s at the end", []() {
			ip_parse_out<ipv6_addr> result = parse_ipv6_helper(L"1234::abcd:8888:dd");

			expect(result.ip).to_have_value(ipv6_addr(0x1234, 0, 0, 0, 0, 0xabcd, 0x8888, 0xdd));
			expect(result.out).to_be(L"1234::abcd:8888:dd");
		});

		it("Parses an IP with five pieces elided and two h16s at the end", []() {
			ip_parse_out<ipv6_addr> result = parse_ipv6_helper(L"1234::8888:dd");

			expect(result.ip).to_have_value(ipv6_addr(0x1234, 0, 0, 0, 0, 0, 0x8888, 0xdd));
			expect(result.out).to_be(L"1234::8888:dd");
		});

		it("Parses an IP with six pieces elided and one h16 at the end", []() {
			ip_parse_out<ipv6_addr> result = parse_ipv6_helper(L"1234::dd");

			expect(result.ip).to_have_value(ipv6_addr(0x1234, 0, 0, 0, 0, 0, 0, 0xdd));
			expect(result.out).to_be(L"1234::dd");
		});

		it("Parses an IP with one piece elided and one h16 at the end", []() {
			ip_parse_out<ipv6_addr> result = parse_ipv6_helper(L"1234:5678:9abc:def0:1234:5678::dd");

			expect(result.ip).to_have_value(ipv6_addr(0x1234, 0x5678, 0x9abc, 0xdef0, 0x1234, 0x5678, 0, 0xdd));
			expect(result.out).to_be(L"1234:5678:9abc:def0:1234:5678::dd");
		});

		it("Parses an IP with the first four pieces elided and two h16s at the end", []() {
			ip_parse_out<ipv6_addr> result = parse_ipv6_helper(L"::1234:5678:9abc:def0");

			expect(result.ip).to_have_value(ipv6_addr(0, 0, 0, 0, 0x1234, 0x5678, 0x9abc, 0xdef0));
			expect(result.out).to_be(L"::1234:5678:9abc:def0");
		});

		it("Parses an IP with the last four pieces elided", []() {
			ip_parse_out<ipv6_addr> result = parse_ipv6_helper(L"1234:5678:9abc:def0::");

			expect(result.ip).to_have_value(ipv6_addr(0x1234, 0x5678, 0x9abc, 0xdef0, 0, 0, 0, 0));
			expect(result.out).to_be(L"1234:5678:9abc:def0::");
		});

		it("Parses an IP with all but the last piece elided and one h16 at the end", []() {
			ip_parse_out<ipv6_addr> result = parse_ipv6_helper(L"::f00");

			expect(result.ip).to_have_value(ipv6_addr(0, 0, 0, 0, 0, 0, 0, 0xf00));
			expect(result.out).to_be(L"::f00");
		});

		it("Parses an IP with all but the first piece elided", []() {
			ip_parse_out<ipv6_addr> result = parse_ipv6_helper(L"f00::");

			expect(result.ip).to_have_value(ipv6_addr(0xf00, 0, 0, 0, 0, 0, 0, 0));
			expect(result.out).to_be(L"f00::");
		});

		it("Parses an IP with every piece elided", []() {
			ip_parse_out<ipv6_addr> result = parse_ipv6_helper(L"::");

			expect(result.ip).to_have_value(ipv6_addr{});
			expect(result.out).to_be(L"::");
		});

		it("Parses an IP with no pieces elided and an IPv4 at the end", []() {
			ip_parse_out<ipv6_addr> result = parse_ipv6_helper(L"1234:0191:f:1:2929:abcd:136.136.0.221");

			expect(result.ip).to_have_value(ipv6_addr(0x1234, 0x0191, 0xf, 0x1, 0x2929, 0xabcd, 0x8888, 0xdd));
			expect(result.out).to_be(L"1234:0191:f:1:2929:abcd:136.136.0.221");
		});

		it("Parses an IP with one piece elided and an IPv4 at the end", []() {
			ip_parse_out<ipv6_addr> result = parse_ipv6_helper(L"1234:0191::1:2929:abcd:192.168.0.1");

			expect(result.ip).to_have_value(ipv6_addr(0x1234, 0x0191, 0, 0x1, 0x2929, 0xabcd, 0xc0a8, 0x1));
			expect(result.out).to_be(L"1234:0191::1:2929:abcd:192.168.0.1");
		});

		it("Parses an IP with two pieces elided and an IPv4 at the end", []() {
			ip_parse_out<ipv6_addr> result = parse_ipv6_helper(L"1234::1:2929:abcd:0.0.0.0");

			expect(result.ip).to_have_value(ipv6_addr(0x1234, 0, 0, 0x1, 0x2929, 0xabcd, 0, 0));
			expect(result.out).to_be(L"1234::1:2929:abcd:0.0.0.0");
		});

		it("Parses an IP with three pieces elided and an IPv4 at the end", []() {
			ip_parse_out<ipv6_addr> result = parse_ipv6_helper(L"1234::2929:abcd:10.0.0.1");

			expect(result.ip).to_have_value(ipv6_addr(0x1234, 0, 0, 0, 0x2929, 0xabcd, 0x0a00, 0x1));
			expect(result.out).to_be(L"1234::2929:abcd:10.0.0.1");
		});

		it("Parses an IP with four pieces elided and an IPv4 at the end", []() {
			ip_parse_out<ipv6_addr> result = parse_ipv6_helper(L"1234::abcd:192.168.0.1");

			expect(result.ip).to_have_value(ipv6_addr(0x1234, 0, 0, 0, 0, 0xabcd, 0xc0a8, 0x1));
			expect(result.out).to_be(L"1234::abcd:192.168.0.1");
		});

		it("Parses an IP with all but the last piece elided and an IPv4 at the end", []() {
			ip_parse_out<ipv6_addr> result = parse_ipv6_helper(L"::192.168.0.1");

			expect(result.ip).to_have_value(ipv6_addr(0, 0, 0, 0, 0, 0, 0xc0a8, 0x1));
			expect(result.out).to_be(L"::192.168.0.1");
		});

		it("Parses an IP with five pieces elided and an IPv4 at the end", []() {
			ip_parse_out<ipv6_addr> result = parse_ipv6_helper(L"1234::192.168.0.1");

			expect(result.ip).to_have_value(ipv6_addr(0x1234, 0, 0, 0, 0, 0, 0xc0a8, 0x1));
			expect(result.out).to_be(L"1234::192.168.0.1");
		});

		it("Parses an IP with two h16s at the end even when garbage follows", []() {
			ip_parse_out<ipv6_addr> result = parse_ipv6_helper(L"1:22:333:4444:5:6:7:8:aaaa");

			expect(result.ip).to_have_value(ipv6_addr(0x1, 0x22, 0x333, 0x4444, 0x5, 0x6, 0x7, 0x8));
			expect(result.out).to_be(L"1:22:333:4444:5:6:7:8");
		});

		it("Parses an IP with an IPv4 at the end even when garbage follows", []() {
			ip_parse_out<ipv6_addr> result = parse_ipv6_helper(L"1:22:333:4444:5:6:192.168.0.1:aaaa");

			expect(result.ip).to_have_value(ipv6_addr(0x1, 0x22, 0x333, 0x4444, 0x5, 0x6, 0xc0a8, 0x1));
			expect(result.out).to_be(L"1:22:333:4444:5:6:192.168.0.1");
		});

		it("Parses an IP with one piece elided and two h16s at the end even when garbage follows", []() {
			ip_parse_out<ipv6_addr> result = parse_ipv6_helper(L"1:22:333:4444::6:7:8:9:aaaa");

			expect(result.ip).to_have_value(ipv6_addr(0x1, 0x22, 0x333, 0x4444, 0, 0x6, 0x7, 0x8));
			expect(result.out).to_be(L"1:22:333:4444::6:7:8");
		});

		it("Parses an IP with three pieces elided and an IPv4 at the end even when garbage follows", []() {
			ip_parse_out<ipv6_addr> result = parse_ipv6_helper(L"1:22::6:192.168.0.1:aaaa");

			expect(result.ip).to_have_value(ipv6_addr(0x1, 0x22, 0, 0, 0, 0x6, 0xc0a8, 0x1));
			expect(result.out).to_be(L"1:22::6:192.168.0.1");
		});
	});
}