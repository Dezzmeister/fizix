#include "logging.h"

void logger::init() {
	try {
		platform::enable_stdout_colors();
	} catch (platform::api_error err) {
		error(std::string(err.what()));
	}
}