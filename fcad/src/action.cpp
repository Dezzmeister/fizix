#include <sstream>
#include <util.h>
#include "action.h"
#include "actions/create.h"

void action_impl::on_accept(char) {}
void action_impl::on_reject(char) {}
void action_impl::on_continue(char) {}

char_seq_action::char_seq_action(
	const std::string &_char_seq,
	action_impl &_impl
) :
	char_seq(_char_seq),
	impl(_impl)
{
	if (char_seq.empty()) {
		throw action_error(
			"Cannot create a char_seq_action with "
			"an empty character sequence"
		);
	}
}

action_state char_seq_action::test(char c) {
	if (char_seq[curr_pos] != c) {
		impl.on_reject(c);
		return action_state::Reject;
	}

	if (curr_pos == char_seq.size() - 1) {
		curr_pos = 0;

		impl.on_accept(c);
		return action_state::Accept;
	}

	curr_pos++;

	impl.on_continue(c);
	return action_state::Continue;
}

std::string char_seq_action::to_string(size_t) const {
	return "char_seq_action(" + char_seq + ")";
}

action_group::action_group(std::vector<std::unique_ptr<action>> &&_actions) :
	actions(std::move(_actions))
{
	if (actions.empty()) {
		throw action_error(
			"Cannot create an action_group with "
			"no actions"
		);
	}
}

action_state action_group::test(char c) {
	if (active_action) {
		action_state out = active_action->test(c);

		if (out != action_state::Continue) {
			active_action = nullptr;
		}

		return out;
	}

	action_state out = action_state::Reject;

	for (auto &act : actions) {
		action_state state = act->test(c);

		if (state != action_state::Reject) {
			if (out != action_state::Reject) {
				throw action_error(
					"Ambiguous actions in action group"
				);
			}

			out = state;
		}
	}

	return out;
}

std::string action_group::to_string(size_t indent) const {
	std::stringstream out{};

	out << "action_group(\n";

	for (size_t i = 0; i < actions.size(); i++) {
		out << std::string(indent + 1, '\t') << traits::to_string(*actions[i], indent);

		if (i != actions.size() - 1) {
			out << ",";
		}

		out << "\n";
	}

	out << std::string(indent, '\t') << ")";

	return out.str();
}

action_tree::action_tree(
	const char_seq_action &_trunk,
	action_group &_leaves,
	action_impl &_trunk_impl
) :
	trunk(_trunk),
	leaves(std::move(_leaves)),
	trunk_impl(_trunk_impl)
{}

action_state action_tree::test(char c) {
	if (! in_leaves) {
		action_state out = trunk.test(c);

		switch (out) {
			case action_state::Accept: {
				trunk_impl.on_accept(c);
				in_leaves = true;

				return action_state::Continue;
			}
			case action_state::Reject: {
				trunk_impl.on_reject(c);
				break;
			}
			case action_state::Continue: {
				trunk_impl.on_continue(c);
				break;
			}
		}

		return out;
	}

	action_state out = leaves.test(c);

	if (out != action_state::Continue) {
		in_leaves = false;
	}

	return out;
}

std::string action_tree::to_string(size_t indent) const {
	return util::obj_to_string(
		"action_tree",
		indent,
		util::named_val("trunk", trunk),
		util::named_val("leaves", leaves)
	);
}

template <>
std::string traits::to_string(const action_state &state, size_t) {
	switch (state) {
		case action_state::Reject:
			return "action_state::Reject";
		case action_state::Continue:
			return "action_state::Continue";
		case action_state::Accept:
			return "action_state::Accept";
		default:
			std::unreachable();
	}
}

template <>
std::string traits::to_string(const action &act, size_t indent) {
	return act.to_string(indent);
}

template <>
std::string traits::to_string(const char_seq_action &act, size_t indent) {
	return act.to_string(indent);
}

template <>
std::string traits::to_string(const action_group &act, size_t indent) {
	return act.to_string(indent);
}

template<>
std::string traits::to_string(const action_tree &act, size_t indent) {
	return act.to_string(indent);
}

window_actions::window_actions(
	action_group &&_actions,
	std::vector<std::unique_ptr<action_impl>> &&_action_impls
) :
	actions(std::move(_actions)),
	action_impls(std::move(_action_impls))
{}

window_actions make_window_actions(event_buses&, fcad_event_bus &events) {
	std::unique_ptr<action_impl> create_vertex = std::make_unique<create_vertex_impl>(
		events
	);
	std::unique_ptr<action_impl> create_edge = std::make_unique<create_edge_impl>(
		events
	);
	std::unique_ptr<action_impl> create_face = std::make_unique<create_face_impl>(
		events
	);

	std::unique_ptr<action> new_feature_actions[] = {
		std::make_unique<char_seq_action>("v", *create_vertex),
		std::make_unique<char_seq_action>("e", *create_edge),
		std::make_unique<char_seq_action>("f", *create_face)
	};

	std::unique_ptr<action_impl> action_impls[] = {
		std::move(create_vertex),
		std::move(create_edge),
		std::move(create_face)
	};

	action_group actions(
		std::vector<std::unique_ptr<action>>(
			std::make_move_iterator(std::begin(new_feature_actions)),
			std::make_move_iterator(std::end(new_feature_actions))
		)
	);

	return window_actions(
		std::move(actions),
		std::vector<std::unique_ptr<action_impl>>(
			std::make_move_iterator(std::begin(action_impls)),
			std::make_move_iterator(std::end(action_impls))
		)
	);
}