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
	geom->set_vert_labels_visible(! geom->are_vert_labels_visible());
}