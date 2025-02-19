#pragma once
#include <coroutine>
#include <experimental/generator>
#include <ranges>
#include <stdexcept>
#include <string>

namespace util {
	template <typename T>
	concept numeric = std::integral<T> || std::floating_point<T>;

	template <typename T>
	concept not_numeric = ! numeric<T>;

	template <typename T, const size_t N>
	constexpr size_t c_arr_size(const T(&)[N]) {
		return N;
	}

	template <typename Val>
	struct str_kv_pair {
		const char key[256];
		const Val val;
	};

	// This can be used to implement a rudimentary compile-time map. Create a constexpr array
	// of `str_kv_pair` and call this function in a constexpr context. See shader_constants.h
	// for an example.
	template <typename Val, const size_t N, const size_t M>
	constexpr Val find_in_map(const str_kv_pair<Val>(&pairs)[N], const char(&key)[M]) {
		for (size_t i = 0; i < N; i++) {
			bool found = true;

			for (size_t j = 0; j < M; j++) {
				if (pairs[i].key[j] != key[j]) {
					found = false;
					break;
				}
			}

			if (found) {
				return pairs[i].val;
			}
		}

		throw std::logic_error("Key not in map");
	}

	constexpr float epsilon = 1e-6f;

	template <numeric T>
	bool eq_within_epsilon(const T &a, const T &b) {
		return std::abs(a - b) <= (T)epsilon;
	}

	template <typename T>
	bool eq_within_epsilon(const T &a, const T &b) {
		for (typename T::length_type i = 0; i < T::length(); i++) {
			if (! eq_within_epsilon(a[i], b[i])) {
				return false;
			}
		}

		return true;
	}

	template <typename LeftView, typename RightView>
	requires std::same_as<std::ranges::range_value_t<LeftView>, std::ranges::range_value_t<RightView>>
	std::experimental::generator<std::ranges::range_value_t<LeftView>> concat_views(LeftView l, RightView r) {
		auto it1 = std::begin(l);

		while (it1 != std::end(l)) {
			co_yield *it1;
			++it1;
		}

		auto it2 = std::begin(r);

		while (it2 != std::end(r)) {
			co_yield *it2;
			++it2;
		}
	}

	inline std::wstring to_wstring(const std::string &s) {
		return std::wstring(s.begin(), s.end());
	}
};
