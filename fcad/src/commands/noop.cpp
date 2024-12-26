#include "commands.h"

void noop_command_impl::on_cancel(const std::wstring&) {}
void noop_command_impl::on_input(const std::wstring&) {}
void noop_command_impl::on_submit(const std::wstring&) {}