#include "commands.h"
#include "helpers.h"

void labels_command_impl::on_submit(const std::wstring &args) {
	const std::wstring trimmed_args = trim(args);

	if (args == L"on") {
		geom->set_vert_labels_visible(true);
	} else if (args == L"off") {
		geom->set_vert_labels_visible(false);
	}
}