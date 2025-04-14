#include "commands.h"
#include "helpers.h"

void labeltype_command_impl::on_submit(const std::wstring &args) {
	const std::wstring trimmed_args = trim(args);

	if (trimmed_args == L"index") {
		geom->set_vert_label_type(vert_label_type::IndexOnly);
	} else if (trimmed_args == L"pos") {
		geom->set_vert_label_type(vert_label_type::IndexAndPos);
	} else if (trimmed_args == L"off") {
		geom->set_vert_label_type(vert_label_type::None);
	}
}

void labeltype_command_impl::write_help_text(std::ostream &os) const {
	write_help_rtf_row(os, ":labeltype (index|pos|off)",
		"Sets the vertex label type. {\\f1\\b index} shows only the vertex index; "
		"{\\f1\\b pos} shows the vertex index and position; {\\f1\\b off} disables "
		"label vertices."
	);
}