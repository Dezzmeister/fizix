#pragma once
#include "action.h"
#include "controllers/edit_history.h"

class start_command_impl : public noop_action_impl {
public:
	using noop_action_impl::noop_action_impl;

	void on_accept(char c) override;
};

class undo_edit_impl : public noop_action_impl {
public:
	using noop_action_impl::noop_action_impl;

	void on_accept(char c) override;
};

class redo_edit_impl : public noop_action_impl {
public:
	using noop_action_impl::noop_action_impl;

	void on_accept(char c) override;
};

class toggle_labels_impl : public noop_action_impl {
public:
	using noop_action_impl::noop_action_impl;

	void on_accept(char c) override;
};

class yank_face_impl : public noop_action_impl {
	using noop_action_impl::noop_action_impl;

	void on_accept(char c) override;
};

class paste_impl : public noop_action_impl {
	using noop_action_impl::noop_action_impl;

	void on_accept(char c) override;
};