#include "controllers/clipboard.h"
#include "controllers/geometry.h"

clipboard_controller::clipboard_controller(fcad_event_bus &_events) :
	event_listener<fcad_start_event>(&_events),
	events(_events)
{
	event_listener<fcad_start_event>::subscribe();
}

const polyhedron& clipboard_controller::add_poly(const std::wstring &name, const polyhedron &p) {
	polys[name] = p;

	return polys.at(name);
}

const polyhedron * clipboard_controller::get_poly(const std::wstring &name) const {
	auto it = polys.find(name);

	if (it != std::end(polys)) {
		return &it->second;
	}

	return nullptr;
}

void clipboard_controller::remove_poly(const std::wstring &name) {
	polys.erase(name);
}

int clipboard_controller::handle(fcad_start_event &event) {
	geom = &event.gc;

	return 0;
}

const std::wstring& clipboard_controller::selection_name() {
	static const std::wstring name = L"<sel>";

	return name;
}