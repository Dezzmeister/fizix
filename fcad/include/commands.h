#pragma once
#include "command.h"
#include "controllers/camera.h"
#include "controllers/edit_history.h"
#include "controllers/file.h"
#include "controllers/geometry.h"
#include "fcad_events.h"

class noop_command_impl :
	public command_impl,
	public event_listener<fcad_start_event>
{
public:
	noop_command_impl(fcad_event_bus &_events);

	void on_cancel(const std::wstring &args_buf) override;
	void on_input(const std::wstring &args_buf) override;
	void on_submit(const std::wstring &args) override;

	int handle(fcad_start_event &event) override;

protected:
	fcad_event_bus &events;
	edit_history_controller * history{};
	file_controller * files{};
	geometry_controller * geom{};
	camera_controller * camera{};
};

class create_vertex_command_impl : public noop_command_impl {
public:
	using noop_command_impl::noop_command_impl;

	void on_submit(const std::wstring &args) override;
};

class create_edge_command_impl : public noop_command_impl {
public:
	using noop_command_impl::noop_command_impl;

	void on_submit(const std::wstring &args) override;
};

class create_face_command_impl : public noop_command_impl {
public:
	using noop_command_impl::noop_command_impl;

	void on_submit(const std::wstring &args) override;
};

class delete_vertex_command_impl : public noop_command_impl {
public:
	using noop_command_impl::noop_command_impl;

	void on_submit(const std::wstring &args) override;
};

class delete_edge_command_impl : public noop_command_impl {
public:
	using noop_command_impl::noop_command_impl;

	void on_submit(const std::wstring &args) override;
};

class delete_face_command_impl : public noop_command_impl {
public:
	using noop_command_impl::noop_command_impl;

	void on_submit(const std::wstring &args) override;
};

class write_file_command_impl : public noop_command_impl {
public:
	using noop_command_impl::noop_command_impl;

	void on_submit(const std::wstring &args) override;

private:
	std::optional<std::wstring> active_file{};
};

class read_file_command_impl : public noop_command_impl {
public:
	using noop_command_impl::noop_command_impl;

	void on_submit(const std::wstring &args) override;
};

class focus_command_impl : public noop_command_impl {
public:
	using noop_command_impl::noop_command_impl;

	void on_submit(const std::wstring &args) override;
};

class quit_command_impl : public noop_command_impl {
public:
	using noop_command_impl::noop_command_impl;

	void on_submit(const std::wstring &args) override;
};

class labels_command_impl : public noop_command_impl {
public:
	using noop_command_impl::noop_command_impl;

	void on_submit(const std::wstring &args) override;
};

class labeltype_command_impl : public noop_command_impl {
public:
	using noop_command_impl::noop_command_impl;

	void on_submit(const std::wstring &args) override;
};