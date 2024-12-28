#include <cassert>
#include "helpers.h"

std::wstring trim(const std::wstring &s) {
	int start = 0;

	while (start < s.size() && (
		s[start] == L' ' ||
		s[start] == L'\t' ||
		s[start] == L'\n' ||
		s[start] == L'\r' ||
		s[start] == L'\v'
	)) {
		start++;
	}

	if (start == s.size()) {
		return L"";
	}

	int end = (int)s.size() - 1;

	while (end >= 0 && (
		s[end] == L' ' ||
		s[end] == L'\t' ||
		s[end] == L'\n' ||
		s[end] == L'\r' ||
		s[end] == L'\v'
	)) {
		end--;
	}

	assert(end >= 0);

	return s.substr(start, end - start + 1);
}