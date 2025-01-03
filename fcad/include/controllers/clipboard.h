#pragma once
#include <map>
#include "fcad_events.h"

class clipboard_controller :
	traits::pinned<clipboard_controller>,
	event_listener<fcad_start_event>
{
public:
	clipboard_controller(fcad_event_bus &_events);

	const polyhedron& add_poly(const std::wstring &name, const polyhedron &p);
	const polyhedron * get_poly(const std::wstring &name) const;
	void remove_poly(const std::wstring &name);

	int handle(fcad_start_event &event) override;

	static const std::wstring& selection_name();

private:
	fcad_event_bus &events;
	geometry_controller * geom{};
	std::map<std::wstring, polyhedron> polys{};
};