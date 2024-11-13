#include <iomanip>
#include "gui.h"
#include "tools.h"
#include "shader_store.h"
#include "draw2d.h"
#include "texture_store.h"

using namespace std::literals::chrono_literals;

gui::gui(event_buses &_buses, custom_event_bus &_custom_bus) :
	event_listener<program_start_event>(&_buses.lifecycle),
	event_listener<pre_render_pass_event>(&_buses.render),
	event_listener<post_processing_event>(&_buses.render),
	event_listener<tool_register_event>(&_custom_bus),
	buses(_buses)
{
	event_listener<program_start_event>::subscribe();
	event_listener<pre_render_pass_event>::subscribe();
	event_listener<post_processing_event>::subscribe();
	event_listener<tool_register_event>::subscribe();
}

int gui::handle(program_start_event &event) {
	debug_font = &event.draw2d->get_font("spleen_6x12");
	selected_tool_bg = &event.textures->store("selected_tool_bg", "assets/icons/selected-item-background.png");
	unselected_tool_bg = &event.textures->store("unseletec_tool_bg", "assets/icons/item-background.png");

	return 0;
}

int gui::handle(pre_render_pass_event&) {
	std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();

	if (now - last_fps_update > 1s) {
		long long millis = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_fps_update).count();

		fps = ((float)millis * frames) / 1000.0f;

		last_fps_update = now;
		frames = 0;
	}

	frames++;

	return 0;
}

int gui::handle(post_processing_event &event) {
	draw_crosshair(event);
	draw_fps_count(event);
	draw_tools(event);

	return 0;
}

int gui::handle(tool_register_event &event) {
	tools.push_back(event.t);

	return 0;
}

void gui::draw_fps_count(const post_processing_event &event) const {
	std::stringstream ss{};
	ss << std::setprecision(4) << " FPS: " << fps;

	event.draw2d.draw_text(
		ss.str(),
		*debug_font,
		0, 12,
		0, 0, 0,
		glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
		glm::vec4(1.0f, 1.0f, 1.0f, 0.7f)
	);
}

void gui::draw_crosshair(const post_processing_event &event) const {
	event.draw2d.draw_text(
		"o",
		*debug_font,
		(event.screen_width - debug_font->glyph_width) / 2,
		(event.screen_height + debug_font->glyph_height) / 2,
		0, 0, 0,
		glm::vec4(0.3f, 0.3f, 1.0f, 1.0f),
		glm::vec4(0.0f)
	);
}

void gui::draw_tools(const post_processing_event &event) const {
	if (! tools.size()) {
		return;
	}

	// We assume that all icons are the same size
	const glm::uvec2 &dimensions = tools[0]->icon.get_dimensions();
	const int spacing = 20;
	const int height_needed = (int)((dimensions.y * tools.size()) + (spacing * (tools.size() - 1)));
	const int available_height = event.screen_height - 100;
	float scale_f = 100.0f / dimensions.y;

	if (height_needed > available_height) {
		scale_f = available_height / (float)height_needed;
	}

	int y = (int)((available_height - (height_needed * scale_f)) / 2);
	int x = (int)(event.screen_width - (dimensions.x * scale_f));
	int width = (int)(dimensions.x * scale_f);
	int height = (int)(dimensions.y * scale_f);
	int scaled_spacing = (int)(spacing * scale_f);

	for (const tool * t : tools) {
		const texture * bg = t->is_active() ? selected_tool_bg : unselected_tool_bg;

		event.draw2d.draw_icon(
			*bg,
			x, y,
			width, height
		);

		event.draw2d.draw_icon(
			t->icon,
			x, y,
			width, height
		);

		y += height + scaled_spacing;

		if (t->is_active()) {
			draw_tool_info(event, t);
		}
	}
}

void gui::draw_tool_info(const post_processing_event &event, const tool * t) const {
	int width = 280;
	int height = 100;
	int title_height = debug_font->glyph_height;
	int sw = 3;

	event.draw2d.draw_rect(
		event.screen_width - width,
		event.screen_height - height,
		width,
		height,
		glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
	);

	event.draw2d.draw_rect(
		event.screen_width - width + sw,
		event.screen_height - height + sw,
		width - (sw * 2),
		height - (sw * 2),
		glm::vec4(1.0f, 1.0f, 0.917f, 1.0f)
	);

	event.draw2d.draw_rect(
		event.screen_width - width,
		event.screen_height - height + title_height + sw,
		width,
		sw,
		glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
	);

	int title_width = (int)(t->name.size()) * debug_font->glyph_width;

	event.draw2d.draw_text(
		t->name,
		*debug_font,
		event.screen_width - width + (width - title_width) / 2,
		event.screen_height - height + title_height + sw,
		width,
		debug_font->glyph_height,
		0,
		glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
		glm::vec4(0.0f)
	);

	event.draw2d.draw_text(
		t->tooltip,
		*debug_font,
		event.screen_width - width + sw + 2,
		event.screen_height - height + (2 * sw) + title_height + debug_font->glyph_height,
		width - (2 * sw) - 2,
		height - title_height - (3 * sw),
		0,
		glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
		glm::vec4(0.0f)
	);
}
