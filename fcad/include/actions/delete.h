#pragma once
#include "action.h"

class delete_vertex_impl : public noop_action_impl {
public:
	using noop_action_impl::noop_action_impl;

	void on_accept(char c) override;
};

class delete_edge_impl : public noop_action_impl {
public:
	using noop_action_impl::noop_action_impl;

	void on_accept(char c) override;
};

class delete_face_impl : public noop_action_impl {
public:
	using noop_action_impl::noop_action_impl;

	void on_accept(char c) override;
};