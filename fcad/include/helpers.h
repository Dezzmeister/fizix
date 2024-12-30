#pragma once
#include <iostream>
#include <stdexcept>

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
