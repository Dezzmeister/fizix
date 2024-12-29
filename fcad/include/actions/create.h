#pragma once
#include "action.h"

class create_vertex_impl : public noop_action_impl {
public:
	using noop_action_impl::noop_action_impl;

	void on_accept(char c) override;
};

class create_edge_impl : public noop_action_impl {
public:
	using noop_action_impl::noop_action_impl;

	void on_accept(char c) override;
};

class create_face_impl : public noop_action_impl {
public:
	using noop_action_impl::noop_action_impl;

	void on_accept(char c) override;
};
