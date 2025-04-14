#include <cassert>
#include "helpers.h"

namespace {
	const int tbl_width_twips = 13600;
	const int name_twips = 3000;
	const int desc_twips = tbl_width_twips - name_twips;
}

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

void write_help_rtf_row(
	std::ostream &os,
	const std::string &name,
	const std::string &desc
) {
	os << R"({\trowd\trgaph180\clvertalc\cellx)" << name_twips;
	os << R"(\cellx)" << desc_twips;
	os << R"(\pard\intbl\ql\f1\fs14 {\b )" << name << R"(}\cell)";
	os << R"(\pard\intbl\ql\f0\fs20 )" << desc << R"(\cell\row})";
}

void write_help_rtf_header(std::ostream &os, const std::string &header) {
	os << R"({\pard\qc\fs38\sb180 )";
	os << header;
	os << R"(\par})";
}

std::string fmt_vec3(const phys::vec3 &v) {
	return std::format("({} {} {})", v.x, v.y, v.z);
}
