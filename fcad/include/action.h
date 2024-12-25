#pragma once
#include <events.h>
#include <memory>
#include <stdexcept>
#include <vector>
#include <traits.h>
#include "fcad_events.h"

enum class action_state {
	Accept,
	Reject,
	Continue
};

class action {
public:
	virtual ~action() = default;

	virtual action_state test(char c) = 0;
	virtual std::string to_string(size_t indent) const = 0;
};

class action_impl {
public:
	virtual ~action_impl() = default;

	virtual void on_accept(char c);
	virtual void on_reject(char c);
	virtual void on_continue(char c);
};

class char_seq_action : public action {
public:
	char_seq_action(
		const std::string &_char_seq,
		action_impl &_impl
	);

	action_state test(char c) override;

	std::string to_string(size_t indent) const override;

private:
	const std::string char_seq;
	action_impl &impl;
	int curr_pos{};
};

class action_group : public action {
public:
	action_group(std::vector<std::unique_ptr<action>> &&_actions);

	action_state test(char c) override;

	std::string to_string(size_t indent) const override;

private:
	std::vector<std::unique_ptr<action>> actions;
	action * active_action{};
};

class action_tree : public action {
public:
	action_tree(
		const char_seq_action &_trunk,
		action_group &_leaves,
		action_impl &_trunk_impl
	);

	action_state test(char c) override;

	std::string to_string(size_t indent) const override;

private:
	char_seq_action trunk;
	action_group leaves;
	action_impl &trunk_impl;
	bool in_leaves{};
};

class action_error : public std::runtime_error {
	using std::runtime_error::runtime_error;
};

class window_actions {
public:
	action_group actions;

	window_actions(
		action_group &&_actions,
		std::vector<std::unique_ptr<action_impl>> &&_action_impls
	);
private:
	std::vector<std::unique_ptr<action_impl>> action_impls;
};

window_actions make_window_actions(event_buses &buses, fcad_event_bus &events);

namespace traits {
	template <>
	std::string to_string(const action_state &state, size_t indent);

	template <>
	std::string to_string(const action &act, size_t indent);

	template <>
	std::string to_string(const char_seq_action &act, size_t indent);

	template <>
	std::string to_string(const action_group &act, size_t indent);

	template<>
	std::string to_string(const action_tree &act, size_t indent);
}