#pragma once
#include "command.h"
#include "controllers/camera.h"
#include "controllers/clipboard.h"
#include "controllers/edit_history.h"
#include "controllers/file.h"
#include "controllers/geometry.h"
#include "fcad_events.h"
#include "fcad_platform/platform.h"

class noop_command_impl :
	public command_impl,
	public event_listener<fcad_start_event>
{
public:
	noop_command_impl(fcad_event_bus &_events);

	void on_cancel(const std::wstring &args_buf) override;
	void on_input(const std::wstring &args_buf) override;
	void on_submit(const std::wstring &args) override;
	void write_help_text(std::ostream &os) const override;

	int handle(fcad_start_event &event) override;

protected:
	fcad_event_bus &events;
	edit_history_controller * history{};
	file_controller * files{};
	geometry_controller * geom{};
	camera_controller * camera{};
	platform_bridge * platform{};
	clipboard_controller * clipboard{};
};

struct create_vertex_preview :
	traits::pinned<create_vertex_preview>
{
	mesh preview;
	mat4 base_transform;
	bool is_visible{};

	create_vertex_preview(
		mesh &&_preview,
		const mat4 &_base_transform = glm::identity<glm::mat4>()
	);

	void show(world &w, const vec3 &at = vec3(0.0f));
	void hide(world &w);
};

class create_vertex_command_impl : 
	traits::pinned<create_vertex_command_impl>,
	public noop_command_impl
{
public:
	create_vertex_command_impl(fcad_event_bus &_events);

	void on_cancel(const std::wstring &args_buf) override;
	void on_input(const std::wstring &args_buf) override;
	void on_submit(const std::wstring &args) override;
	void write_help_text(std::ostream &os) const override;

	int handle(fcad_start_event &event) override;

private:
	world * mesh_world{};
	geometry vert_geom;
	geometry line_geom;
	create_vertex_preview vert_preview;
	create_vertex_preview line_preview;
	create_vertex_preview plane_preview;
};

class create_edge_command_impl : public noop_command_impl {
	using noop_command_impl::noop_command_impl;

	void on_submit(const std::wstring &args) override;
	void write_help_text(std::ostream &os) const override;
};

class create_face_command_impl : public noop_command_impl {
	using noop_command_impl::noop_command_impl;

	void on_submit(const std::wstring &args) override;
	void write_help_text(std::ostream &os) const override;
};

class delete_vertex_command_impl : public noop_command_impl {
	using noop_command_impl::noop_command_impl;

	void on_submit(const std::wstring &args) override;
	void write_help_text(std::ostream &os) const override;
};

class delete_edge_command_impl : public noop_command_impl {
	using noop_command_impl::noop_command_impl;

	void on_submit(const std::wstring &args) override;
	void write_help_text(std::ostream &os) const override;
};

class delete_face_command_impl : public noop_command_impl {
	using noop_command_impl::noop_command_impl;

	void on_submit(const std::wstring &args) override;
	void write_help_text(std::ostream &os) const override;
};

class write_file_command_impl : public noop_command_impl {
	using noop_command_impl::noop_command_impl;

	void on_submit(const std::wstring &args) override;
	void write_help_text(std::ostream &os) const override;
};

class read_file_command_impl : public noop_command_impl {
	using noop_command_impl::noop_command_impl;

	void on_submit(const std::wstring &args) override;
	void write_help_text(std::ostream &os) const override;
};

class focus_command_impl : public noop_command_impl {
	using noop_command_impl::noop_command_impl;

	void on_submit(const std::wstring &args) override;
	void write_help_text(std::ostream &os) const override;
};

class quit_command_impl : public noop_command_impl {
	using noop_command_impl::noop_command_impl;

	void on_submit(const std::wstring &args) override;
	void write_help_text(std::ostream &os) const override;
};

class labels_command_impl : public noop_command_impl {
	using noop_command_impl::noop_command_impl;

	void on_submit(const std::wstring &args) override;
	void write_help_text(std::ostream &os) const override;
};

class labeltype_command_impl : public noop_command_impl {
	using noop_command_impl::noop_command_impl;

	void on_submit(const std::wstring &args) override;
	void write_help_text(std::ostream &os) const override;
};

class help_command_impl : public noop_command_impl {
	using noop_command_impl::noop_command_impl;

	int handle(fcad_start_event &event) override;

	void on_submit(const std::wstring &args) override;
	void write_help_text(std::ostream &os) const override;

private:
	// TODO: Help URL just in case
	std::string help_text{ "Something went wrong" };
};

class flip_command_impl : public noop_command_impl {
	using noop_command_impl::noop_command_impl;

	void on_submit(const std::wstring &args) override;
	void write_help_text(std::ostream &os) const override;
};

class yank_face_command_impl : public noop_command_impl {
	using noop_command_impl::noop_command_impl;

	void on_submit(const std::wstring &args) override;
	void write_help_text(std::ostream &os) const override;
};

class yank_group_command_impl : public noop_command_impl {
	using noop_command_impl::noop_command_impl;

	void on_submit(const std::wstring &args) override;
	void write_help_text(std::ostream &os) const override;
};

class paste_command_impl : public noop_command_impl {
	using noop_command_impl::noop_command_impl;

	void on_submit(const std::wstring &args) override;
	void write_help_text(std::ostream &os) const override;
};

class move_group_command_impl : public noop_command_impl {
	using noop_command_impl::noop_command_impl;

	void on_submit(const std::wstring &args) override;
	void write_help_text(std::ostream &os) const override;
};

class delete_group_command_impl : public noop_command_impl {
	using noop_command_impl::noop_command_impl;

	void on_submit(const std::wstring &args) override;
	void write_help_text(std::ostream &os) const override;
};

class vertex_info_command_impl : public noop_command_impl {
	using noop_command_impl::noop_command_impl;

	void on_submit(const std::wstring &args) override;
	void write_help_text(std::ostream &os) const override;
};