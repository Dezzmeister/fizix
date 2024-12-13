#include "controllers.h"
#include "draw2d.h"
#include "gdi_plus_context.h"
#include "gl.h"
#include "hardware_constants.h"
#include "logging.h"
#include "mesh.h"
#include "player.h"
#include "shader_store.h"
#include "shapes.h"
#include "texture_store.h"
#include "util.h"
#include "world.h"

// TODO: Check command line and possibly load file
int main(int, const char * const * const) {
	logger::init();
	platform::state platform_state{};
	platform::window main_window(platform_state, 800, 800, L"FCAD");

	event_buses buses;
	gdi_plus_context gdi_plus;
	main_window.show();
	main_window.make_gl_context_current();
	init_gl();

	glViewport(0, 0, 800, 800);
	// Old Windows BSOD blue
	glClearColor(0.031f, 0.152f, 0.961f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	// TODO: Custom camera
	player pl(buses);
	hardware_constants hw_consts(buses);

	program_start_event program_start(&main_window);
	pre_render_pass_event pre_render_event(&main_window, &hw_consts);
	// TODO: Make built-in shaders and textures optional
	shader_store shaders(buses);
	texture_store textures(buses);
	renderer2d draw2d(buses);
	draw_event draw_event_inst(&main_window, shaders, textures);
	post_processing_event post_processing_event_inst(
		&main_window,
		shaders,
		textures,
		draw2d
	);
	post_render_pass_event post_render_event{};

	key_controller keys(buses, {
		KEY_ESC
	});
	// TODO: Disable mouse locking
	mouse_controller mouse(buses, {}, KEY_ESC);
	screen_controller screen(buses);

	world w(buses);

	buses.lifecycle.fire(program_start);

	shapes::init();

	main_window.run([&](platform::window &win) {
		buses.render.fire(pre_render_event);
		buses.render.fire(draw_event_inst);
		buses.render.fire(post_processing_event_inst);

		win.swap_buffers();

		buses.render.fire(post_render_event);
	});

	program_stop_event program_stop(0);
	buses.lifecycle.fire(program_stop);

	return 0;
}