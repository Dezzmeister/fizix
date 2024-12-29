#include "actions/misc.h"

void start_command_impl::on_accept(char) {
	mode->set_mode(edit_mode::Command, L":");
}

void undo_edit_impl::on_accept(char) {
	history->undo();
}

void redo_edit_impl::on_accept(char) {
	history->redo();
}