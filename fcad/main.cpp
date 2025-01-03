// Use Common Controls V6 so that cue banners will work
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#include "fcad_platform/platform.h"
#include <controllers.h>
#include <draw2d.h>
#include <gdi_plus_context.h>
#include <gl.h>
#include <helpers.h>
#include <hardware_constants.h>
#include <logging.h>
#include <mesh.h>
#include <player.h>
#include <shader_store.h>
#include <shapes.h>
#include <texture_store.h>
#include <util.h>
#include <world.h>
#include "action.h"
#include "controllers/action.h"
#include "controllers/camera.h"
#include "controllers/clipboard.h"
#include "controllers/command.h"
#include "controllers/edit_history.h"
#include "fcad_events.h"
#include "controllers/file.h"
#include "controllers/geometry.h"
#include "controllers/mode.h"
#include "controllers/preferences.h"

// TODO: Check command line and possibly load file
int main(int, const char * const * const) {
	logger::init();
	platform::enable_fp_exceptions();
	platform::state platform_state{};
	platform::window main_window(platform_state, 800, 800, L"FCAD");

	main_window.show();
	main_window.make_gl_context_current();
	shapes::init();

	event_buses buses;
	fcad_event_bus events;
	mode_controller mode(buses, events);
	platform_bridge platform(
		events,
		platform_state,
		main_window,
		mode
	);
	window_actions actions = make_window_actions(buses, events);
	action_controller ac(events, actions.actions);
	command_controller commands = make_commands(buses, events);
	edit_history_controller edit_history(events);
	file_controller fc(events);
	gdi_plus_context gdi_plus;

	glViewport(0, 0, 800, 800);
	// Old Windows BSOD blue
	glClearColor(0.031f, 0.152f, 0.961f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

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
		KEY_ESC,
		KEY_CTRL,
		KEY_H,
		KEY_J,
		KEY_K,
		KEY_L,
		KEY_B,
		KEY_PERIOD,
		KEY_I,
		KEY_O,
		KEY_X,
		KEY_Y,
		KEY_Z
	});
	mouse_controller mouse(buses, {}, -1);
	screen_controller screen(buses);
	camera_controller camera(buses, events);
	world mesh_world(buses);
	geometry_controller geom(buses, events, mesh_world);
	preferences_controller prefs(events);
	clipboard_controller clipboard(events);
	fcad_start_event fcad_start(
		platform,
		geom,
		edit_history,
		fc,
		mode,
		camera,
		ac,
		commands,
		prefs,
		mesh_world,
		clipboard
	);

	platform.set_cue_text(L"Type :h and press ENTER for help");

	buses.lifecycle.fire(program_start);
	events.fire(fcad_start);

	platform_state.run([&]() {
		buses.render.fire(pre_render_event);
		buses.render.fire(draw_event_inst);
		buses.render.fire(post_processing_event_inst);

		main_window.swap_buffers();

		buses.render.fire(post_render_event);
	});

	program_stop_event program_stop(0);
	buses.lifecycle.fire(program_stop);

	return 0;
}