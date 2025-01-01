#include "controllers/file.h"
#include "controllers/preferences.h"

preferences_controller::preferences_controller(
	fcad_event_bus &_events
) :
	event_listener<fcad_start_event>(&_events, 50)
{
	event_listener<fcad_start_event>::subscribe();
}

int preferences_controller::handle(fcad_start_event &event) {
	wchar_t * buf{};
	size_t num_elems{};

	if (_wdupenv_s(&buf, &num_elems, L"USERPROFILE") || ! buf) {
		logger::error("Failed to get %USERPROFILE% env variable");
		return 0;
	}

	std::wstring userprofile(buf);
	free(buf);

	std::filesystem::path base_dir(userprofile);

	event.fc.read_file(base_dir / L".fcadrc", false);

	return 0;
}
