#pragma once
#include "action.h"
#include "controllers/edit_history.h"

class start_command_impl : public action_impl {
public:
	start_command_impl(fcad_event_bus &_events);

	void on_accept(char c) override;

private:
	fcad_event_bus &events;
};

class undo_edit_impl :
	public action_impl,
	public event_listener<fcad_start_event>
{
public:
	undo_edit_impl(fcad_event_bus &_events);

	void on_accept(char c) override;

	int handle(fcad_start_event &event) override;

private:
	edit_history_controller * history;
};

class redo_edit_impl :
	public action_impl,
	public event_listener<fcad_start_event>
{
public:
	redo_edit_impl(fcad_event_bus &_events);

	void on_accept(char c) override;

	int handle(fcad_start_event &event) override;

private:
	edit_history_controller * history;
};