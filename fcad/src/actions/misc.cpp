#include "actions/misc.h"
#include "controllers/edit_history.h"
#include "controllers/geometry.h"
#include "controllers/mode.h"

void start_command_impl::on_accept(char) {
	mode->set_mode(edit_mode::Command, L":");
}

void undo_edit_impl::on_accept(char) {
	history->undo();
}

void redo_edit_impl::on_accept(char) {
	history->redo();
}

void toggle_labels_impl::on_accept(char) {
	switch (geom->get_vert_label_type()) {
		case vert_label_type::IndexAndPos:
			geom->set_vert_label_type(vert_label_type::IndexOnly);
			break;
		case vert_label_type::IndexOnly:
			geom->set_vert_label_type(vert_label_type::None);
			break;
		case vert_label_type::None:
			geom->set_vert_label_type(vert_label_type::IndexAndPos);
			break;
	}
}

void yank_face_impl::on_accept(char) {
	mode->set_mode(edit_mode::Command, L":yf ");
}

void yank_group_impl::on_accept(char) {
	mode->set_mode(edit_mode::Command, L":yg ");
}

void paste_impl::on_accept(char) {
	mode->set_mode(edit_mode::Command, L":p ");
}

void move_group_impl::on_accept(char) {
	mode->set_mode(edit_mode::Command, L":mg ");
}

void delete_group_impl::on_accept(char) {
	mode->set_mode(edit_mode::Command, L":dg ");
}

void vertex_info_impl::on_accept(char) {
	mode->set_mode(edit_mode::Command, L":n ");
}