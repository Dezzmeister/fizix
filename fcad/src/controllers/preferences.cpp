#include "controllers/file.h"
#include "controllers/preferences.h"

preferences_controller::preferences_controller(
	fcad_event_bus &_events
) :
	event_listener<fcad_start_event>(&_events, 50)
{
	event_listener<fcad_start_event>::subscribe();
}

void preferences_controller::set_log_cmd_output(bool _log) {
	_log_cmd_output = _log;
}

bool preferences_controller::log_cmd_output() const {
	return _log_cmd_output;
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

	event.fc.read_file((base_dir / L".fcadrc").wstring(), false);

	return 0;
}
