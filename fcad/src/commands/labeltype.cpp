#include "commands.h"
#include "helpers.h"

void labeltype_command_impl::on_submit(const std::wstring &args) {
	const std::wstring trimmed_args = trim(args);

	if (trimmed_args == L"index") {
		geom->set_vert_label_type(vert_label_type::IndexOnly);
	} else if (trimmed_args == L"pos") {
		geom->set_vert_label_type(vert_label_type::IndexAndPos);
	}
}