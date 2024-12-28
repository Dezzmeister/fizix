#include "shader_store.h"
#include "draw2d.h"
#include "texture_store.h"
#include "util.h"

namespace {
	constexpr util::str_kv_pair<int> text_shader_locs[] = {
		{ "x_offset", 1 },
		{ "y_offset", 2 },
		{ "glyph_width", 3 },
		{ "glyph_height", 4 },
		{ "screen_width", 5 },
		{ "screen_height", 6 },
		{ "font", 7 },
		{ "bg_color", 8 },
		{ "fg_color", 9 },
		{ "color", 10 },
		{ "icon", 11 },
		{ "depth", 12 }
	};
}

font::font(texture _font_bmp, int _glyph_width, int _glyph_height) :
	font_bmp(std::move(_font_bmp)),
	glyph_width(_glyph_width),
	glyph_height(_glyph_height)
{}

renderer2d::renderer2d(event_buses &_buses) :
	event_listener<program_start_event>(&_buses.lifecycle, -99),
	event_listener<screen_resize_event>(&_buses.render),
	buses(_buses),
	text_vao(0, [](unsigned int vao) {
	glDeleteVertexArrays(1, &vao);
		}),
	text_vbo(0, [](unsigned int vbo) {
	glDeleteBuffers(1, &vbo);
		}),
	rect_vao(0, [](unsigned int vao) {
	glDeleteVertexArrays(1, &vao);
		}),
	rect_vbo(0, [](unsigned int vbo) {
	glDeleteBuffers(1, &vbo);
		}),
	icon_vao(0, [](unsigned int vao) {
	glDeleteVertexArrays(1, &vao);
		}),
	icon_vbo(0, [](unsigned int vbo) {
	glDeleteBuffers(1, &vbo);
		})
{
	event_listener<program_start_event>::subscribe();
	event_listener<screen_resize_event>::subscribe();

	glGenVertexArrays(1, &text_vao);
	glBindVertexArray(text_vao);
	glGenBuffers(1, &text_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, text_vbo);
	glVertexAttribIPointer(0, 1, GL_UNSIGNED_BYTE, sizeof(char), 0);
	glEnableVertexAttribArray(0);
	glBufferData(GL_ARRAY_BUFFER, sizeof text_vbo_buf, NULL, GL_STREAM_DRAW);

	glGenVertexArrays(1, &rect_vao);
	glBindVertexArray(rect_vao);
	glGenBuffers(1, &rect_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, rect_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof rect_vbo_buf, NULL, GL_STREAM_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), 0);
	glEnableVertexAttribArray(0);

	glGenVertexArrays(1, &icon_vao);
	glBindVertexArray(icon_vao);
	glGenBuffers(1, &icon_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, icon_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof icon_vbo_buf, NULL, GL_STREAM_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec2), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec2), (void*)sizeof(glm::vec2));
	glEnableVertexAttribArray(1);
}

const font& renderer2d::load_font(const std::string &font_name, const std::string &font_path, int glyph_width, int glyph_height) {
	fonts.insert(std::make_pair(font_name, font(texture(font_path.c_str(), false), glyph_width, glyph_height)));

	const font &f = fonts.at(font_name);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, f.font_bmp.get_id());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	return fonts.at(font_name);
}

const font& renderer2d::get_font(const std::string &font_name) const {
	return fonts.at(font_name);
}

void renderer2d::draw_text(
	const std::string &text,
	const font &f,
	int x,
	int y,
	int max_width,
	int max_height,
	int line_spacing,
	const glm::vec4 &fg_color,
	const glm::vec4 &bg_color,
	bool auto_break,
	float depth
) const {
	static constexpr int glyph_width_loc = util::find_in_map(text_shader_locs, "glyph_width");
	static constexpr int glyph_height_loc = util::find_in_map(text_shader_locs, "glyph_height");
	static constexpr int screen_width_loc = util::find_in_map(text_shader_locs, "screen_width");
	static constexpr int screen_height_loc = util::find_in_map(text_shader_locs, "screen_height");
	static constexpr int font_loc = util::find_in_map(text_shader_locs, "font");
	static constexpr int fg_color_loc = util::find_in_map(text_shader_locs, "fg_color");
	static constexpr int bg_color_loc = util::find_in_map(text_shader_locs, "bg_color");
	static constexpr int depth_loc = util::find_in_map(text_shader_locs, "depth");

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, f.font_bmp.get_id());

	text_shader->use();
	text_shader->set_uniform(glyph_width_loc, f.glyph_width);
	text_shader->set_uniform(glyph_height_loc, f.glyph_height);
	text_shader->set_uniform(screen_width_loc, screen_width);
	text_shader->set_uniform(screen_height_loc, screen_height);
	text_shader->set_uniform(font_loc, 0);
	text_shader->set_uniform(fg_color_loc, fg_color);
	text_shader->set_uniform(bg_color_loc, bg_color);
	text_shader->set_uniform(depth_loc, depth);

	shader_use_event shader_event(*text_shader);
	buses.render.fire(shader_event);

	glBindVertexArray(text_vao);
	glBindBuffer(GL_ARRAY_BUFFER, text_vbo);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	size_t in_char_pos = 0;
	size_t out_char_pos = 0;
	int draw_x = x;
	int curr_x = x;
	int curr_y = y;

	while (in_char_pos < text.size()) {
		bool text_too_tall = max_height && ((curr_y - y) > max_height);
		bool text_too_long = max_width && (curr_x - x + f.glyph_width) > max_width;
		bool text_buf_full = out_char_pos == util::c_arr_size(text_vbo_buf);

		if (text_too_tall) {
			break;
		}

		if (text_too_long) {
			if (! out_char_pos) {
				break;
			}

			if (! auto_break) {
				draw_line(out_char_pos, draw_x, curr_y);
				draw_x = x;
				curr_x = x;
				curr_y += f.glyph_height + line_spacing;
				out_char_pos = 0;
			} else {
				if (text[in_char_pos] == ' ') {
					draw_line(out_char_pos, draw_x, curr_y);
					draw_x = x;
					curr_x = x;
					curr_y += f.glyph_height + line_spacing;
					out_char_pos = 0;
					// Skip the space, effectively replacing it with a newline
					in_char_pos++;
					continue;
				}

				long prev_space = ((long)out_char_pos) - 1;

				while (prev_space != -1 && text_vbo_buf[prev_space--] != ' ');

				if (prev_space == -1 && text_vbo_buf[0] != ' ') {
					// The word is too long, we'll have to break the word with a hyphen
					char next_char = text_vbo_buf[out_char_pos - 1];
					text_vbo_buf[out_char_pos - 1] = '-';

					draw_line(out_char_pos, draw_x, curr_y);
					draw_x = x;
					curr_x = x + f.glyph_width;
					curr_y += f.glyph_height + line_spacing;
					out_char_pos = 1;

					text_vbo_buf[0] = next_char;
				} else {
					prev_space++;
					draw_line(prev_space, draw_x, curr_y);

					memmove(text_vbo_buf, text_vbo_buf + prev_space + 1, out_char_pos - prev_space - 1);
					draw_x = x;
					curr_y += f.glyph_height + line_spacing;

					if (prev_space + 1 < (long)out_char_pos) {
						out_char_pos -= (prev_space + 1);
						curr_x = x + ((int)out_char_pos * f.glyph_width);
					} else {
						out_char_pos = 0;
						curr_x = x;
					}
				}
			}

			continue;
		}

		if (text_buf_full) {
			draw_line(out_char_pos, draw_x, curr_y);
			curr_x += (int)(out_char_pos * f.glyph_width);
			draw_x = curr_x;
			out_char_pos = 0;
			continue;
		}

		const char c = text[in_char_pos];

		if (c == '\n') {
			draw_line(out_char_pos, draw_x, curr_y);
			draw_x = x;
			curr_x = x;
			curr_y += f.glyph_height + line_spacing;
			out_char_pos = 0;
			in_char_pos++;
		} else if (c < ' ' || c > '~') {
			in_char_pos++;
		} else {
			text_vbo_buf[out_char_pos++] = c;
			curr_x += f.glyph_width;
			in_char_pos++;
		}
	}

	if (in_char_pos == text.size() && out_char_pos) {
		draw_line(out_char_pos, draw_x, curr_y);
	}

	glDisable(GL_BLEND);
}

void renderer2d::draw_rect(
	int x,
	int y,
	int width,
	int height,
	const glm::vec4 &color
) const {
	static constexpr int color_loc = util::find_in_map(text_shader_locs, "color");

	rect_vbo_buf[0] = screen_to_gl(glm::ivec2(x, y));
	rect_vbo_buf[1] = screen_to_gl(glm::ivec2(x, y + height));
	rect_vbo_buf[2] = screen_to_gl(glm::ivec2(x + width, y + height));
	rect_vbo_buf[3] = screen_to_gl(glm::ivec2(x + width, y + height));
	rect_vbo_buf[4] = screen_to_gl(glm::ivec2(x + width, y));
	rect_vbo_buf[5] = screen_to_gl(glm::ivec2(x, y));

	rect_shader->use();
	rect_shader->set_uniform(color_loc, color);

	shader_use_event shader_event(*rect_shader);
	buses.render.fire(shader_event);

	glBindVertexArray(rect_vao);
	glBindBuffer(GL_ARRAY_BUFFER, rect_vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof rect_vbo_buf, rect_vbo_buf);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}

void renderer2d::draw_icon(
	const texture &icon,
	int x,
	int y,
	int width,
	int height
) const {
	static constexpr int icon_loc = util::find_in_map(text_shader_locs, "icon");

	icon_vbo_buf[0] = screen_to_gl(glm::ivec2(x, y));
	icon_vbo_buf[2] = screen_to_gl(glm::ivec2(x, y + height));
	icon_vbo_buf[4] = screen_to_gl(glm::ivec2(x + width, y + height));
	icon_vbo_buf[6] = screen_to_gl(glm::ivec2(x + width, y + height));
	icon_vbo_buf[8] = screen_to_gl(glm::ivec2(x + width, y));
	icon_vbo_buf[10] = screen_to_gl(glm::ivec2(x, y));

	icon_vbo_buf[1] = glm::vec2(0.0f, 1.0f);
	icon_vbo_buf[3] = glm::vec2(0.0f, 0.0f);
	icon_vbo_buf[5] = glm::vec2(1.0f, 0.0f);
	icon_vbo_buf[7] = glm::vec2(1.0f, 0.0f);
	icon_vbo_buf[9] = glm::vec2(1.0f, 1.0f);
	icon_vbo_buf[11] = glm::vec2(0.0f, 1.0f);

	icon_shader->use();

	shader_use_event shader_event(*icon_shader);
	buses.render.fire(shader_event);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, icon.get_id());
	icon_shader->set_uniform(icon_loc, 0);

	glBindVertexArray(icon_vao);
	glBindBuffer(GL_ARRAY_BUFFER, icon_vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof icon_vbo_buf, icon_vbo_buf);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}

void renderer2d::draw_line(size_t num_chars, int x, int y) const {
	static constexpr int x_offset_loc = util::find_in_map(text_shader_locs, "x_offset");
	static constexpr int y_offset_loc = util::find_in_map(text_shader_locs, "y_offset");

	glBufferSubData(GL_ARRAY_BUFFER, 0, num_chars * sizeof(char), text_vbo_buf);
	text_shader->set_uniform(x_offset_loc, x);
	text_shader->set_uniform(y_offset_loc, y);

	glDrawArrays(GL_POINTS, 0, (GLsizei)num_chars);
}

int renderer2d::handle(program_start_event &event) {
	event.draw2d = this;

	text_shader = &event.shaders->shaders.at("text2d");
	rect_shader = &event.shaders->shaders.at("rect2d");
	icon_shader = &event.shaders->shaders.at("icon2d");

	screen_width = event.screen_width;
	screen_height = event.screen_height;

	load_font("spleen_6x12", "assets/fonts/spleen_font_6x12.png", 6, 12);
	load_font("spleen_12x24", "assets/fonts/spleen_font_12x24.png", 12, 24);

	return 0;
}

int renderer2d::handle(screen_resize_event &event) {
	screen_width = event.new_width;
	screen_height = event.new_height;

	return 0;
}

glm::ivec2 renderer2d::ndc_to_screen(const glm::vec3 &ndc) const {
	int x = (int)((ndc.x + 1.0f) * screen_width / 2.0f);
	int y = screen_height - (int)((ndc.y + 1.0f) * screen_height / 2.0f);

	return glm::ivec2(x, y);
}

glm::vec2 renderer2d::screen_to_gl(const glm::ivec2 &v) const {
	float x = 2.0f * (v.x - screen_width / 2.0f) / screen_width;
	float y = 2.0f * (screen_height / 2.0f - v.y) / screen_height;

	return glm::vec2(x, y);
}