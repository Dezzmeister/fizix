#include <fstream>
#include <sstream>
#include "data_formats/json.h"
#include "test.h"

using namespace test;

static const wchar_t complicated_example[] = LR"(
{
	"simple_key_1": 1,
	"simple_key_2":			"some_string",
	"obj_key_1": {



		"simple_key_3": true,
		"simple_key_4": null,
"arr_key_1": [
			1, 2, 3, "four", "five", {
				"simple_key_5": 1e-6
			}
		]}}
)";

static const wchar_t escape_sequences[] = LR"(
[
	"newline: \n",
	"tab: \t",
	"cr: \r",
	"formfeed: \f",
	"backspace: \b",
	"forward slash: \/",
	"backslash: \\",
	"unicode BEL: \u0007",
	"unicode space: \u0020",
	"unicode beef: \ubEeF"
]
)";

static const wchar_t extra_comma_failure[] =
LR"({
	"key1": [1, 2, 3, 4, 5],
	"key2": {
		"    another key      ": 15,
		"": null
	},
})";

void setup_json_parser_tests() {
	describe("json", []() {
		it("Parses a complicated JSON string", []() {
			std::wstringstream wss(complicated_example);
			json result = parse_json(wss);

			json_value_or_descriptor root = result.get_root();
			json_object_descriptor * obj_desc = nullptr;
			json_object * obj = nullptr;
			json_array_descriptor * arr_desc = nullptr;
			json_array * arr = nullptr;

			expect_msg("Root node is an object descriptor", (obj_desc = std::get_if<json_object_descriptor>(&root)));
			expect_msg("Root node is a valid object", (obj = result.get(*obj_desc).value_or(nullptr)));
			expect_msg("Root object has 3 properties", obj->size() == 3);
			expect_msg("root.simple_key_1 is defined", obj->contains(L"simple_key_1"));
			expect_msg("root.simple_key_2 is defined", obj->contains(L"simple_key_2"));
			expect_msg("root.obj_key_1 is defined", obj->contains(L"obj_key_1"));
			expect_msg("root.simple_key_1 is 1", obj->at(L"simple_key_1") == json_value_or_descriptor(1L));
			expect_msg("root.simple_key_2 is \"some_string\"", obj->at(L"simple_key_2") == json_value_or_descriptor(L"some_string"));
			expect_msg("root.obj_key_1 is an object descriptor", (obj_desc = std::get_if<json_object_descriptor>(&(*obj)[L"obj_key_1"])));
			expect_msg("root.obj_key_1 is a valid object", (obj = result.get(*obj_desc).value_or(nullptr)));
			expect_msg("root.obj_key_1 has 3 properties", obj->size() == 3);
			expect_msg("root.obj_key_1.simple_key_3 is defined", obj->contains(L"simple_key_3"));
			expect_msg("root.obj_key_1.simple_key_4 is defined", obj->contains(L"simple_key_4"));
			expect_msg("root.obj_key_1.arr_key_1 is defined", obj->contains(L"arr_key_1"));
			expect_msg("root.obj_key_1.simple_key_3 is 'true'", obj->at(L"simple_key_3") == json_value_or_descriptor(true));
			expect_msg("root.obj_key_1.simple_key_4 is 'null'", obj->at(L"simple_key_4") == json_value_or_descriptor(nullptr));
			expect_msg("root.obj_key_1.arr_key_1 is an array descriptor", (arr_desc = std::get_if<json_array_descriptor>(&(*obj)[L"arr_key_1"])));
			expect_msg("root.obj_key_1.arr_key_1 is a valid array", (arr = result.get(*arr_desc).value_or(nullptr)));
			expect_msg("root.obj_key_1.arr_key_1 has 6 elements", arr->size() == 6);
			expect_msg("root.obj_key_1.arr_key_1[0] is 1", arr->at(0) == json_value_or_descriptor(1L));
			expect_msg("root.obj_key_1.arr_key_1[1] is 2", arr->at(1) == json_value_or_descriptor(2L));
			expect_msg("root.obj_key_1.arr_key_1[2] is 3", arr->at(2) == json_value_or_descriptor(3L));
			expect_msg("root.obj_key_1.arr_key_1[3] is \"four\"", arr->at(3) == json_value_or_descriptor(L"four"));
			expect_msg("root.obj_key_1.arr_key_1[4] is \"five\"", arr->at(4) == json_value_or_descriptor(L"five"));
			expect_msg("root.obj_key_1.arr_key_1[5] is an object descriptor", (obj_desc = std::get_if<json_object_descriptor>(&(*arr)[5])));
			expect_msg("root.obj_key_1.arr_key_1[5] is a valid object", (obj = result.get(*obj_desc).value_or(nullptr)));
			expect_msg("root.obj_key_1.arr_key_1[5] has 1 property", obj->size() == 1);
			expect_msg("root.obj_key_1.arr_key_1[5].simple_key_5 is defined", obj->contains(L"simple_key_5"));
			expect_msg("root.obj_key_1.arr_key_1[5].simple_key_5 is 1e-6", obj->at(L"simple_key_5") == json_value_or_descriptor(1e-6));
		});

		it("Parses a JSON file", []() {
			std::wifstream wif(L"assets/tests/json_parser_test.json");
			json result = parse_json(wif);

			expect_cond(result.num_objects() == 12);
			expect_cond(result.num_arrays() == 1);
		});

		it("Parses escape sequences", []() {
			std::wstringstream wss(escape_sequences);
			json result = parse_json(wss);
			json_value_or_descriptor root = result.get_root();
			json_array_descriptor * arr_desc = nullptr;
			json_array * arr = nullptr;

			expect_msg("Root node is an array descriptor", (arr_desc = std::get_if<json_array_descriptor>(&root)));
			expect_msg("Root node is a valid array", (arr = result.get(*arr_desc).value_or(nullptr)));
			expect_msg("Root node has 10 elements", arr->size() == 10);
			expect_msg("0th element is correct", arr->at(0) == json_value_or_descriptor(L"newline: \n"));
			expect_msg("1st element is correct", arr->at(1) == json_value_or_descriptor(L"tab: \t"));
			expect_msg("2nd element is correct", arr->at(2) == json_value_or_descriptor(L"cr: \r"));
			expect_msg("3rd element is correct", arr->at(3) == json_value_or_descriptor(L"formfeed: \f"));
			expect_msg("4th element is correct", arr->at(4) == json_value_or_descriptor(L"backspace: \b"));
			expect_msg("5th element is correct", arr->at(5) == json_value_or_descriptor(L"forward slash: /"));
			expect_msg("6th element is correct", arr->at(6) == json_value_or_descriptor(L"backslash: \\"));
			expect_msg("7th element is correct", arr->at(7) == json_value_or_descriptor(L"unicode BEL: \u0007"));
			expect_msg("8th element is correct", arr->at(8) == json_value_or_descriptor(L"unicode space:  "));
			expect_msg("9th element is correct", arr->at(9) == json_value_or_descriptor(L"unicode beef: \ubeef"));
		});

		it("Parses a string with a non-ASCII character", []() {
			std::wstringstream wss(L"\"this contains unicode: \ubeef\"");
			json result = parse_json(wss);

			expect_cond(result.get_root() == json_value_or_descriptor(L"this contains unicode: \ubeef"));
		});

		it("Parses an empty object", []() {
			std::wstringstream wss(L"{}");
			json result = parse_json(wss);

			json_value_or_descriptor root = result.get_root();
			json_object_descriptor * obj_desc = nullptr;
			json_object * obj = nullptr;

			expect_msg("Root node is an object descriptor", (obj_desc = std::get_if<json_object_descriptor>(&root)));
			expect_msg("Root node is a valid object", (obj = result.get(*obj_desc).value_or(nullptr)));
			expect_msg("Root object is empty", obj->size() == 0);
		});

		it("Parses an empty array", []() {
			std::wstringstream wss(L"[]");
			json result = parse_json(wss);

			json_value_or_descriptor root = result.get_root();
			json_array_descriptor * arr_desc = nullptr;
			json_array * arr = nullptr;

			expect_msg("Root node is an array descriptor", (arr_desc = std::get_if<json_array_descriptor>(&root)));
			expect_msg("Root node is a valid array", (arr = result.get(*arr_desc).value_or(nullptr)));
			expect_msg("Root array is empty", arr->size() == 0);
		});

		it("Parses one string", []() {
			std::wstringstream wss(L"   \r\t\n\n   \"one lone string, but still valid json\"\n\n\t\r\t");
			json result = parse_json(wss);

			expect_cond(result.get_root() == json_value_or_descriptor(L"one lone string, but still valid json"));
		});

		it("Parses one integer", []() {
			std::wstringstream wss(L"      \r\t\n      -789                 ");
			json result = parse_json(wss);

			expect_cond(result.get_root() == json_value_or_descriptor(-789L));
		});

		it("Parses one double", []() {
			std::wstringstream wss(L"     \n\n\n\n\n\n\r\r\r\r\r\r       8.76e2");
			json result = parse_json(wss);

			expect_cond(result.get_root() == json_value_or_descriptor(8.76e2));
		});

		it("Parses 'true'", []() {
			std::wstringstream wss(L"    true      ");
			json result = parse_json(wss);

			expect_cond(result.get_root() == json_value_or_descriptor(true));
		});

		it("Parses 'false'", []() {
			std::wstringstream wss(L"false \t\t\n");
			json result = parse_json(wss);

			expect_cond(result.get_root() == json_value_or_descriptor(false));
		});

		it("Parses 'null'", []() {
			std::wstringstream wss(L"null");
			json result = parse_json(wss);

			expect_cond(result.get_root() == json_value_or_descriptor(nullptr));
		});

		it("Parses zero", []() {
			std::wstringstream wss(L"0");
			json result = parse_json(wss);

			expect_cond(result.get_root() == json_value_or_descriptor(0L));
		});

		it("Fails when an object contains an extra comma", []() {
			std::wstringstream wss(extra_comma_failure);

			try {
				json result = parse_json(wss);

				fail_msg("Expected the parser to throw");
			} catch (json_parse_error err) {
				expect_cond(err.line_num == 7);
				expect_cond(err.col_num == 1);
				expect_cond(std::string(err.what()).find("Expected a quoted string") != std::string::npos);
			}
		});

		it("Fails when a string contains a control character", []() {
			std::wstringstream wss(L"[\"this string contains a \n newline\"]");

			try {
				json result = parse_json(wss);

				fail_msg("Expected the parser to throw");
			} catch (json_parse_error err) {
				expect_cond(err.line_num == 2);
				expect_cond(err.col_num == 0);
			}
		});

		it("Fails when an integer contains a leading zero", []() {
			std::wstringstream wss(L"05");

			try {
				json result = parse_json(wss);

				fail_msg("Expected the parser to throw");
			} catch (json_parse_error err) {
				expect_cond(err.line_num == 1);
				expect_cond(err.col_num == 1);
			}
		});
	});
}