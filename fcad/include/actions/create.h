#pragma once
#include "action.h"

class create_vertex_impl : public action_impl {
public:
	create_vertex_impl(fcad_event_bus &_events);

	void on_accept(char c) override;

private:
	fcad_event_bus &events;
};

class create_edge_impl : public action_impl {
public:
	create_edge_impl(fcad_event_bus &_events);

	void on_accept(char c) override;

private:
	fcad_event_bus &events;
};

class create_face_impl : public action_impl {
public:
	create_face_impl(fcad_event_bus &_events);

	void on_accept(char c) override;

private:
	fcad_event_bus &events;
};
