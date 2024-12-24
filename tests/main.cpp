#include <float.h>
#include "test.h"

extern void setup_json_parser_tests();
extern void setup_base64_tests();
extern void setup_ipaddr_tests();
extern void setup_uri_tests();
extern void setup_bvh_tests();
extern void setup_collision_tests();
extern void setup_geometry_tests();

int main(int, const char * const * const) {
#pragma warning(push)
#pragma warning(disable: 4996)
	_controlfp(EM_DENORMAL | EM_UNDERFLOW | EM_INEXACT, _MCW_EM);
#pragma warning(pop)

	setup_json_parser_tests();
	setup_base64_tests();
	setup_ipaddr_tests();
	setup_uri_tests();
	setup_bvh_tests();
	setup_collision_tests();
	setup_geometry_tests();

	test::run();

	return 0;
}