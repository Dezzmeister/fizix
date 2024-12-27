#pragma once
#include "command.h"
#include "fcad_events.h"

class noop_command_impl : public command_impl {
public:
	// TODO: Does this need to be redeclared as virtual?
	virtual ~noop_command_impl() = default;

	void on_cancel(const std::wstring &args_buf) override;
	void on_input(const std::wstring &args_buf) override;
	void on_submit(const std::wstring &args) override;
};

class create_vertex_command_impl : public noop_command_impl {
public:
	create_vertex_command_impl(fcad_event_bus &_events);

	void on_submit(const std::wstring &args) override;

private:
	fcad_event_bus &events;
};

class create_edge_command_impl : public noop_command_impl {
public:
	create_edge_command_impl(fcad_event_bus &_events);

	void on_submit(const std::wstring &args) override;

private:
	fcad_event_bus &events;
};

class create_face_command_impl : public noop_command_impl {
public:
	create_face_command_impl(fcad_event_bus &_events);

	void on_submit(const std::wstring &args) override;

private:
	fcad_event_bus &events;
};

class delete_vertex_command_impl : public noop_command_impl {
public:
	delete_vertex_command_impl(fcad_event_bus &_events);

	void on_submit(const std::wstring &args) override;

private:
	fcad_event_bus &events;
};

class focus_command_impl : public noop_command_impl {
public:
	focus_command_impl(fcad_event_bus &_events);

	void on_submit(const std::wstring &args) override;

private:
	fcad_event_bus &events;
};

class quit_command_impl : public noop_command_impl {
public:
	void on_submit(const std::wstring &args) override;
};