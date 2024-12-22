#include <logging.h>
#include <util.h>
#include "commands.h"
#include "parsing.h"

using namespace phys;

struct face_indices {
	std::optional<std::vector<size_t>> indices{ std::vector<size_t>{} };

	face_indices(const std::wstring &args) {
		std::wstringstream wss{};
		std::wstringstream sink{};

		wss << args;

		parsing::parser_state state(wss);

		while (! state.eof()) {
			std::optional<size_t> idx_opt = parse_size(state);

			if (! idx_opt) {
				indices = std::nullopt;
				return;
			}

			indices->push_back(*idx_opt);

			parsing::parse_one_char(state, L',', sink);
			parsing::parse_whitespace(state);
		}
	}
};

create_face_command_impl::create_face_command_impl(fcad_event_bus &_events) :
	events(_events) {}

void create_face_command_impl::on_cancel(const std::wstring&) {}
void create_face_command_impl::on_input(const std::wstring&) {}
void create_face_command_impl::on_submit(const std::wstring &args) {
	face_indices face_opt(args);

	if (! face_opt.indices) {
		return;
	}

	face f(*face_opt.indices);
	new_face_event event(f);
	events.fire(event);
}
