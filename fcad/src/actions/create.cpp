#include "actions/create.h"

create_vertex_impl::create_vertex_impl(fcad_event_bus &_events) :
	events(_events) {}

void create_vertex_impl::on_accept(char) {
	set_mode_event event(edit_mode::Command, L":v ");
	events.fire(event);
}

create_edge_impl::create_edge_impl(fcad_event_bus &_events) :
	events(_events) {}

void create_edge_impl::on_accept(char) {
	set_mode_event event(edit_mode::Command, L":e ");
	events.fire(event);
}

create_face_impl::create_face_impl(fcad_event_bus &_events) :
	events(_events) {}

void create_face_impl::on_accept(char) {
	set_mode_event event(edit_mode::Command, L":f ");
	events.fire(event);
}