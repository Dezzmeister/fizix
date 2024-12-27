#pragma once
#include "action.h"

class delete_vertex_impl : public action_impl {
public:
	delete_vertex_impl(fcad_event_bus &_events);

	void on_accept(char c) override;

private:
	fcad_event_bus &events;
};

class delete_edge_impl : public action_impl {
public:
	delete_edge_impl(fcad_event_bus &_events);

	void on_accept(char c) override;

private:
	fcad_event_bus &events;
};

class delete_face_impl : public action_impl {
public:
	delete_face_impl(fcad_event_bus &_events);

	void on_accept(char c) override;

private:
	fcad_event_bus &events;
};