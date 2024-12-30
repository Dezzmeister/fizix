#pragma once
#include <string>

class command_impl {
public:
	virtual ~command_impl() = default;

	virtual void on_cancel(const std::wstring &args_buf) = 0;
	virtual void on_input(const std::wstring &args_buf) = 0;
	virtual void on_submit(const std::wstring &args) = 0;
	virtual void write_help_text(std::ostream &os) const = 0;
};