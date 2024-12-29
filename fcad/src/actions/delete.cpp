#include "actions/delete.h"

void delete_vertex_impl::on_accept(char) {
	mode->set_mode(edit_mode::Command, L":dv ");
}

void delete_edge_impl::on_accept(char) {
	mode->set_mode(edit_mode::Command, L":de ");
}

void delete_face_impl::on_accept(char) {
	mode->set_mode(edit_mode::Command, L":df ");
}