#pragma once
#include <iostream>
#include <stdexcept>
#include "physics/math.h"

std::wstring trim(const std::wstring &s);
void write_help_rtf_row(
	std::ostream &os,
	const std::string &name,
	const std::string &desc
);
void write_help_rtf_header(
	std::ostream &os,
	const std::string &header
);
std::string fmt_vec3(const phys::vec3 &v);
