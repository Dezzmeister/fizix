#pragma once
#include <unordered_map>
#include "events.h"
#include "texture.h"
#include "unique_handle.h"

// A monospace font. Each glyph has the same width and height. At the moment,
// only characters from ASCII space to tilde are supported.
struct font {
	texture font_bmp;
	int glyph_width;
	int glyph_height;

	font(texture _font_bmp, int _glyph_width, int _glyph_height);
};

struct text_metrics {
	const int width;
	const int height;

	text_metrics(int _width, int _height) :
		width(_width),
		height(_height)
	{}
};

// Responsible for drawing simple 2D graphics to the screen.
class renderer2d :
	public event_listener<program_start_event>,
	public event_listener<screen_resize_event>
{
public:
	renderer2d(event_buses &_buses);

	// Loads the font at the given `font_path`, and assigns it `font_name`.
	// The font itself must be a monospace bitmap font with white glyphs on
	// a black background. Only ASCII glyphs from space to tilde are supported.
	const font& load_font(
		const std::string &font_name,
		const std::string &font_path,
		int glyph_width,
		int glyph_height
	);

	// Gets the font with the given name, for use in `draw_text`. This font must have
	// been loaded first with `load_font`. The following fonts are provided by the engine:
	//  spleen_6x12 - The Spleen font, with glyphs 6 pixels wide and 12 pixels high:
	//		https://github.com/fcambus/spleen
	const font& get_font(const std::string &font_name) const;

	// Draws text on the screen at some position, using the given font. The x and y
	// coordinates are interpreted as pixels, with the origin at the top left corner
	// of the screen, so that +x is right and +y is down. The lower-left corner of the
	// first line of rendered text will be at the given position.
	//
	// An optional max width and height can be provided. If the max width is nonzero,
	// `draw_text` will wrap text to the next line so that a line of text does not exceed
	// the given width. No hyphens or other delimiting characters will be inserted. If
	// the max height is nonzero, `draw_text` will not draw any text below `y` + `max_height`.
	// `draw_text` will try to draw all of the given text, but it may run out of space if
	// `max_width` is nonzero. Note that if `max_width` is zero, no text will be wrapped, even
	// at the edge of the screen.
	//
	// If `line_spacing` is zero, each line will be drawn immediately below the next, with 
	// no space between glyphs (most glyphs are shorter in appearance than the glyph height,
	// so this looks fine). `line_spacing` sets the number of pixels to leave between lines.
	// 
	// The text and background can be given colors, in RGBA format.
	//
	// `auto_break` can be set to break lines on spaces, or on very long words. A word will only
	// be broken if it is too long to fit on one line.
	void draw_text(
		const std::string &text,
		const font &f,
		int x,
		int y,
		int max_width,
		int max_height,
		int line_spacing,
		const glm::vec4 &fg_color,
		const glm::vec4 &bg_color,
		bool auto_break = true
	) const;

	void draw_rect(
		int x,
		int y,
		int width,
		int height,
		const glm::vec4 &color
	) const;

	void draw_icon(
		const texture &icon,
		int x,
		int y,
		int width,
		int height
	) const;

	int handle(program_start_event &event) override;
	int handle(screen_resize_event &event) override;

private:
	std::unordered_map<std::string, font> fonts{};
	event_buses &buses;

	unique_handle<unsigned int> text_vao;
	unique_handle<unsigned int> text_vbo;
	mutable char text_vbo_buf[4096]{};
	const shader_program * text_shader{ nullptr };

	unique_handle<unsigned int> rect_vao;
	unique_handle<unsigned int> rect_vbo;
	mutable glm::vec2 rect_vbo_buf[6]{};
	const shader_program * rect_shader{ nullptr };
	const shader_program * icon_shader{ nullptr };

	unique_handle<unsigned int> icon_vao;
	unique_handle<unsigned int> icon_vbo;
	mutable glm::vec2 icon_vbo_buf[12]{};

	int screen_width{ 0 };
	int screen_height{ 0 };

	void draw_line(size_t num_chars, int x, int y) const;

	glm::vec2 screen_to_gl(const glm::ivec2 &v) const;
};
