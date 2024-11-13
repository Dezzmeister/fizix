#pragma once
#include <functional>
#include <memory>
#include <optional>
#include <stack>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#define expect_cond(cond) do { if (! (cond)) { throw test::assertion_failure(std::nullopt, __FILE__, __LINE__); } } while (0)
#define expect_msg(msg, cond) do { if (! (cond)) { throw test::assertion_failure((msg), __FILE__, __LINE__); } } while (0)
#define fail() throw test::forced_failure(std::nullopt, __FILE__, __LINE__)
#define fail_msg(msg) throw test::forced_failure((msg), __FILE__, __LINE__)

namespace test {
	using callback = std::function<void(void)>;

	struct test_count {
		int passed{};
		int failed{};

		test_count(int _passed, int _failed);
		test_count() = default;

		friend test_count operator+(const test_count &a, const test_count &b);
	};

	struct test_tree {
		std::string title{};
		callback before_all{};
		callback before_each{};
		callback test{};
		callback after_each{};
		callback after_all{};
		std::vector<std::unique_ptr<test_tree>> children{};

		int num_tests() const;
		test_count run(int tabs);
	};

	class test_setup_error : public std::runtime_error {
	public:
		using std::runtime_error::runtime_error;
	};

	struct assertion_failure : public std::runtime_error {
		assertion_failure(std::optional<std::string> message, const char * const file, int line) :
			std::runtime_error("Test assertion failed in " + std::string(file) + " (line " + std::to_string(line) + ")" + (message.has_value() ? ": " + message.value() : ""))
		{
#ifdef DEBUG_TEST_FAILURES
			__debugbreak();
#endif
		}
	};

	struct forced_failure : public std::runtime_error {
		forced_failure(std::optional<std::string> message, const char * const file, int line) :
			std::runtime_error("Forced test failure in " + std::string(file) + " (line " + std::to_string(line) + ")" + (message.has_value() ? ": " + message.value() : ""))
		{
#ifdef DEBUG_TEST_FAILURES
			__debugbreak();
#endif
		}
	};

	extern std::vector<std::unique_ptr<test_tree>> suites;
	extern std::stack<test_tree *> curr_suite;

	void describe(const std::string &title, const callback &cb);
	void it(const std::string &title, const callback &cb);
	void before_all(const callback &cb);
	void before_each(const callback &cb);
	void after_each(const callback &cb);
	void after_all(const callback &cb);

	void run();
}
