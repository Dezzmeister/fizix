#include "actions/create.h"

void create_vertex_impl::on_accept(char) {
	mode->set_mode(edit_mode::Command, L":v ");
}

void create_edge_impl::on_accept(char) {
	mode->set_mode(edit_mode::Command, L":e ");
}

void create_face_impl::on_accept(char) {
	mode->set_mode(edit_mode::Command, L":f ");
}