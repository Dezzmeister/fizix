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

			expect(root).to_have_type<json_object_descriptor>().assert_now();

			json_object_descriptor obj_desc = std::get<json_object_descriptor>(root);
			expect(result.get(obj_desc)).naht().to_be_empty().assert_now();

			json_object * obj = result.get(obj_desc).value();
			expect(obj->size()).to_be(3);
			expect(obj->contains(L"simple_key_1")).to_be(true);
			expect(obj->contains(L"simple_key_2")).to_be(true);
			expect(obj->contains(L"obj_key_1")).to_be(true);
			expect(obj->at(L"simple_key_1")).to_be(1L);
			expect(obj->at(L"simple_key_2")).to_be(L"some_string");

			expect(obj->at(L"obj_key_1")).to_have_type<json_object_descriptor>().assert_now();

			const json_object_descriptor obj_desc_2 = std::get<json_object_descriptor>(obj->at(L"obj_key_1"));
			expect(result.get(obj_desc_2)).naht().to_be_empty().assert_now();

			obj = result.get(obj_desc_2).value();
			expect(obj->size()).to_be(3);
			expect(obj->contains(L"simple_key_3")).to_be(true);
			expect(obj->contains(L"simple_key_4")).to_be(true);
			expect(obj->contains(L"arr_key_1")).to_be(true);
			expect(obj->at(L"simple_key_3")).to_be(true);
			expect(obj->at(L"simple_key_4")).to_be(nullptr);

			expect(obj->at(L"arr_key_1")).to_have_type<json_array_descriptor>().assert_now();

			const json_array_descriptor arr_desc = std::get<json_array_descriptor>(obj->at(L"arr_key_1"));
			expect(result.get(arr_desc)).naht().to_be_empty().assert_now();

			json_array * arr = result.get(arr_desc).value();
			expect(arr->size()).to_be(6);
			expect(arr->at(0)).to_be(1L);
			expect(arr->at(1)).to_be(2L);
			expect(arr->at(2)).to_be(3L);
			expect(arr->at(3)).to_be(L"four");
			expect(arr->at(4)).to_be(L"five");

			expect(arr->at(5)).to_have_type<json_object_descriptor>().assert_now();

			const json_object_descriptor &obj_desc_3 = std::get<json_object_descriptor>(arr->at(5));
			expect(result.get(obj_desc_3)).naht().to_be_empty().assert_now();

			obj = result.get(obj_desc_3).value();
			expect(obj->size()).to_be(1);
			expect(obj->contains(L"simple_key_5")).to_be(true);
			expect(obj->at(L"simple_key_5")).to_be(json_value_or_descriptor(1e-6));
		});

		it("Parses a JSON file", []() {
			std::wifstream wif(L"assets/tests/json_parser_test.json");
			json result = parse_json(wif);

			expect(result.num_objects()).to_be(12);
			expect(result.num_arrays()).to_be(1);
		});

		it("Parses escape sequences", []() {
			std::wstringstream wss(escape_sequences);
			json result = parse_json(wss);
			json_value_or_descriptor root = result.get_root();

			expect(root).to_have_type<json_array_descriptor>().assert_now();

			const json_array_descriptor arr_desc = std::get<json_array_descriptor>(root);
			expect(result.get(arr_desc)).naht().to_be_empty().assert_now();

			json_array * arr = result.get(arr_desc).value();
			expect(arr->size()).to_be(10);
			expect(arr->at(0)).to_be(L"newline: \n");
			expect(arr->at(1)).to_be(L"tab: \t");
			expect(arr->at(2)).to_be(L"cr: \r");
			expect(arr->at(3)).to_be(L"formfeed: \f");
			expect(arr->at(4)).to_be(L"backspace: \b");
			expect(arr->at(5)).to_be(L"forward slash: /");
			expect(arr->at(6)).to_be(L"backslash: \\");
			expect(arr->at(7)).to_be(L"unicode BEL: \u0007");
			expect(arr->at(8)).to_be(L"unicode space:  ");
			expect(arr->at(9)).to_be(L"unicode beef: \ubeef");
		});

		it("Parses a string with a non-ASCII character", []() {
			std::wstringstream wss(L"\"this contains unicode: \ubeef\"");
			json result = parse_json(wss);

			expect(result.get_root()).to_be(L"this contains unicode: \ubeef");
		});

		it("Parses an empty object", []() {
			std::wstringstream wss(L"{}");
			json result = parse_json(wss);

			json_value_or_descriptor root = result.get_root();
			expect(root).to_have_type<json_object_descriptor>().assert_now();

			const json_object_descriptor obj_desc = std::get<json_object_descriptor>(root);
			expect(result.get(obj_desc)).naht().to_be_empty().assert_now();

			json_object * obj = result.get(obj_desc).value();
			expect(obj->size()).to_be(0);
		});

		it("Parses an empty array", []() {
			std::wstringstream wss(L"[]");
			json result = parse_json(wss);

			json_value_or_descriptor root = result.get_root();
			expect(root).to_have_type<json_array_descriptor>().assert_now();

			const json_array_descriptor arr_desc = std::get<json_array_descriptor>(root);
			expect(result.get(arr_desc)).naht().to_be_empty().assert_now();

			json_array * arr = result.get(arr_desc).value();
			expect(arr->size()).to_be(0);
		});

		it("Parses one string", []() {
			std::wstringstream wss(L"   \r\t\n\n   \"one lone string, but still valid json\"\n\n\t\r\t");
			json result = parse_json(wss);

			expect(result.get_root()).to_be(L"one lone string, but still valid json");
		});

		it("Parses one integer", []() {
			std::wstringstream wss(L"      \r\t\n      -789                 ");
			json result = parse_json(wss);

			expect(result.get_root()).to_be(-789L);
		});

		it("Parses one double", []() {
			std::wstringstream wss(L"     \n\n\n\n\n\n\r\r\r\r\r\r       8.76e2");
			json result = parse_json(wss);

			expect(result.get_root()).to_be(8.76e2);
		});

		it("Parses 'true'", []() {
			std::wstringstream wss(L"    true      ");
			json result = parse_json(wss);

			expect(result.get_root()).to_be(true);
		});

		it("Parses 'false'", []() {
			std::wstringstream wss(L"false \t\t\n");
			json result = parse_json(wss);

			expect(result.get_root()).to_be(false);
		});

		it("Parses 'null'", []() {
			std::wstringstream wss(L"null");
			json result = parse_json(wss);

			expect(result.get_root()).to_be(nullptr);
		});

		it("Parses zero", []() {
			std::wstringstream wss(L"0");
			json result = parse_json(wss);

			expect(result.get_root()).to_be(0L);
		});

		it("Fails when an object contains an extra comma", []() {
			std::wstringstream wss(extra_comma_failure);

			try {
				json result = parse_json(wss);

				fail("Expected the parser to throw");
			} catch (json_parse_error err) {
				expect(err.line_num).to_be(7);
				expect(err.col_num).to_be(1);
				expect(std::string(err.what())).to_contain("Expected a quoted string");
			}
		});

		it("Fails when a string contains a control character", []() {
			std::wstringstream wss(L"[\"this string contains a \n newline\"]");

			try {
				json result = parse_json(wss);

				fail("Expected the parser to throw");
			} catch (json_parse_error err) {
				expect(err.line_num).to_be(2);
				expect(err.col_num).to_be(0);
			}
		});

		it("Fails when an integer contains a leading zero", []() {
			std::wstringstream wss(L"05");

			try {
				json result = parse_json(wss);

				fail("Expected the parser to throw");
			} catch (json_parse_error err) {
				expect(err.line_num).to_be(1);
				expect(err.col_num).to_be(1);
			}
		});
	});
}