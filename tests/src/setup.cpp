#include <cassert>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <ranges>
#include "matchers.h"
#include "setup.h"

namespace {
	template <typename T>
	auto to_ref_view(std::vector<std::unique_ptr<T>> &vec) {
		return std::ranges::views::transform(vec, [](auto &p) -> T& { return *p; });
	}

	template <typename T>
	auto to_ref_view(const std::vector<std::unique_ptr<T>> &vec) {
		return std::ranges::views::transform(vec, [](const auto &p) -> const T& { return *p; });
	}
}

std::vector<std::unique_ptr<test::test_tree>> test::suites{};
std::stack<test::test_tree *> test::curr_suite{};

test::test_count::test_count(int _passed, int _failed) :
	passed(_passed), failed(_failed) {}

test::test_count test::operator+(const test_count &a, const test_count &b) {
	return test_count(a.passed + b.passed, a.failed + b.failed);
}

int test::test_tree::num_tests() const {
	if (test) {
		return 1;
	}

	int sum = 0;

	for (const test_tree &tree : to_ref_view(children)) {
		sum += tree.num_tests();
	}

	return sum;
}

test::test_count test::test_tree::run(int tabs) {
	test_count out{};

	std::cout << std::string(tabs, '\t') << title << " ";

	if (! test) {
		std::cout << std::endl;
	}

	const auto start = std::chrono::steady_clock::now();

	try {
		if (before_all) {
			before_all();
		}

		if (test) {
			test();

			if (prev_err) {
				const assertion_failure err = prev_err.value();
				prev_err = {};

				throw err;
			}

			out.passed++;
		}

		for (test_tree &tree : to_ref_view(children)) {
			if (before_each) {
				before_each();
			}

			out = out + tree.run(tabs + 1);

			if (after_each) {
				after_each();
			}
		}

		if (after_all) {
			after_all();
		}
	} catch (std::runtime_error err) {
		const auto end = std::chrono::steady_clock::now();
		const std::chrono::nanoseconds time = end - start;
		long long ns = time.count();
		double ms = ((double)ns) / 1'000'000;

		if (! test) {
			std::cout << std::string(tabs, '\t');
		}

		std::cout << "(" << ms << "ms)" << std::endl;

		int total = num_tests();

		out.failed = total - out.passed;
		std::cerr << "\n" << err.what() << "\n" << std::endl;

		return out;
	}

	const auto end = std::chrono::steady_clock::now();
	const std::chrono::nanoseconds time = end - start;
	long long ns = time.count();
	double ms = ((double)ns) / 1'000'000;

	if (! test) {
		std::cout << std::string(tabs, '\t');
	}

	std::cout << "(" << ms << "ms)" << std::endl;

	return out;
}

void test::describe(const std::string &title, const callback &cb) {
	std::unique_ptr<test_tree> curr_tree = std::make_unique<test_tree>();

	curr_tree->title = title;
	curr_suite.push(curr_tree.get());

	cb();

	if (curr_suite.size() == 1) {
		suites.push_back(std::move(curr_tree));
		curr_suite.pop();
		return;
	} else if (curr_suite.empty()) {
		throw test_setup_error("Something went wrong, unexpected empty stack");
	}

	curr_suite.pop();
	curr_suite.top()->children.push_back(std::move(curr_tree));
}

void test::it(const std::string &title, const callback &cb) {
	if (curr_suite.empty()) {
		throw test_setup_error("it() must be called within a describe() block");
	}

	std::unique_ptr<test_tree> tree = std::make_unique<test_tree>();
	tree->title = title;
	tree->test = cb;

	curr_suite.top()->children.push_back(std::move(tree));
}

void test::before_all(const callback &cb) {
	if (curr_suite.empty()) {
		throw test_setup_error("before_all() must be called within a describe() block");
	}

	curr_suite.top()->before_all = cb;
}

void test::before_each(const callback &cb) {
	if (curr_suite.empty()) {
		throw test_setup_error("before_each() must be called within a describe() block");
	}

	curr_suite.top()->before_each = cb;
}

void test::after_each(const callback &cb) {
	if (curr_suite.empty()) {
		throw test_setup_error("after_each() must be called within a describe() block");
	}

	curr_suite.top()->after_each = cb;
}

void test::after_all(const callback &cb) {
	if (curr_suite.empty()) {
		throw test_setup_error("after_all() must be called within a describe() block");
	}

	curr_suite.top()->after_all = cb;
}

void test::run() {
	std::cout << std::fixed << std::setprecision(2);
	test_count total{};

	for (test_tree &suite : to_ref_view(suites)) {
		total = total + suite.run(0);
	}

	std::cout
		<< std::to_string(total.passed)
		<< "/"
		<< std::to_string(total.passed + total.failed)
		<< " tests passed"
		<< std::endl;
}