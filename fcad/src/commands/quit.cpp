#include <platform/platform.h>
#include "commands.h"

void quit_command_impl::on_submit(const std::wstring&) {
	// TODO: Quit in a platform independent way
	PostQuitMessage(0);
}