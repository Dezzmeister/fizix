#include <Windows.h>
#include <chrono>
#include "controllers.h"
#include "custom_events.h"
#include "directional_light.h"
#include "draw2d.h"
#include "events.h"
#include "flashlight.h"
#include "gdi_plus_context.h"
#include "gui.h"
#include "logging.h"
#include "hardware_constants.h"
#include "mesh.h"
#include "object_world.h"
#include "platform/platform.h"
#include "phong_color_material.h"
#include "phong_map_material.h"
#include "physical_particle_emitter.h"
#include "physics/math.h"
#include "player.h"
#include "shader_store.h"
#include "shapes.h"
#include "spotlight.h"
#include "texture_store.h"
#include "tools.h"
#include "util.h"
#include "world.h"

using namespace phys::literals;
using namespace std::literals::chrono_literals;

static const char help_text[] =
R"(Controls:
	Mouse to look around
	WASD to move
	Hold LEFT SHIFT to sprint
	F to toggle flashlight
	Scroll to change the active tool
		While the spawn tool is active:
		LEFT_MOUSE to spawn a particle
		Hold RIGHT_MOUSE + SCROLL to move the cursor
	P to pause the physics simulation
		While the simulation is paused:
		PERIOD to step forward one frame
)";

int main(int, const char * const * const) {
#pragma warning(push)
#pragma warning(disable: 4996)
	_controlfp(EM_DENORMAL | EM_UNDERFLOW | EM_INEXACT, _MCW_EM);
#pragma warning(pop)

	logger::init();
	platform::state platform_state{};
	platform::window main_window(platform_state, 800, 600, L"Physics Demo");
	event_buses buses;
	custom_event_bus custom_bus;
	gdi_plus_context gdi_plus;

	main_window.show();
	main_window.make_gl_context_current();

	glViewport(0, 0, 800, 600);
	glClearColor(0.7f, 0.7f, 1.0f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	player pl(buses);
	hardware_constants hw_consts(buses);

	// TODO: Pass window by reference
	program_start_event program_start(&main_window);
	pre_render_pass_event pre_render_event(&main_window, &hw_consts);
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
		KEY_W,
		KEY_A,
		KEY_S,
		KEY_D,
		KEY_SHIFT,
		KEY_F,
		KEY_ESC,
		KEY_T,
		KEY_P,
		KEY_PERIOD,
		KEY_1,
		KEY_2,
		KEY_3,
		KEY_4
	});
	mouse_controller mouse(buses, {
		MOUSE_LEFT,
		MOUSE_RIGHT
	}, KEY_ESC);
	screen_controller screen(buses);

	gui g(buses, custom_bus);
	world w(buses);
	toolbox tools(buses, custom_bus, w);

	shapes::init();
	init_constants();

	buses.lifecycle.fire(program_start);

	object_world<1000> objects(
		buses,
		custom_bus,
		w,
		KEY_P,
		KEY_PERIOD
	);

	flashlight lc(buses, pl, w, KEY_F);

	logger::info(help_text);

	platform::run([&]() {
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
