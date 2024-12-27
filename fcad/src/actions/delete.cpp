#include "actions/delete.h"

delete_vertex_impl::delete_vertex_impl(fcad_event_bus &_events) :
	events(_events) {}

void delete_vertex_impl::on_accept(char) {
	set_mode_event event(edit_mode::Command, L":dv ");
	events.fire(event);
}

delete_edge_impl::delete_edge_impl(fcad_event_bus &_events) :
	events(_events) {}

void delete_edge_impl::on_accept(char) {
	set_mode_event event(edit_mode::Command, L":de ");
	events.fire(event);
}

delete_face_impl::delete_face_impl(fcad_event_bus &_events) :
	events(_events) {}

void delete_face_impl::on_accept(char) {
	set_mode_event event(edit_mode::Command, L":df ");
	events.fire(event);
}