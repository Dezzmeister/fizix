#include <Windows.h>
#include <GLFW/glfw3.h>
#include <chrono>
#include "shapes.h"
#include "events.h"
#include "flashlight.h"
#include "gdi_plus_context.h"
#include "controllers.h"
#include "hardware_constants.h"
#include "physics/math.h"
#include "mesh.h"
#include "phong_color_material.h"
#include "phong_map_material.h"
#include "physical_particle_emitter.h"
#include "player.h"
#include "directional_light.h"
#include "shader_store.h"
#include "spotlight.h"
#include "draw2d.h"
#include "texture_store.h"
#include "util.h"
#include "world.h"
#include "custom_events.h"
#include "gui.h"
#include "object_world.h"
#include "tools.h"

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

static void on_window_resize(GLFWwindow *, int width, int height) {
	glViewport(0, 0, width, height);
}

int main(int, const char * const * const) {
	event_buses buses;
	custom_event_bus custom_bus;
	gdi_plus_context gdi_plus;
#pragma warning(push)
#pragma warning(disable: 4996)
	_controlfp(EM_DENORMAL | EM_UNDERFLOW | EM_INEXACT, _MCW_EM);
#pragma warning(pop)
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow * window = glfwCreateWindow(800, 600, "Physics Demo", NULL, NULL);
	if (!window) {
		std::cout << "Failed to create window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	init_gl();

	glViewport(0, 0, 800, 600);
	glfwSetFramebufferSizeCallback(window, on_window_resize);
	glClearColor(0.7f, 0.7f, 1.0f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	player pl(buses);
	hardware_constants hw_consts(buses);

	program_start_event program_start{
		window
	};
	pre_render_pass_event pre_render_event(window, &hw_consts);
	shader_store shaders(buses);
	texture_store textures(buses);
	renderer2d draw2d(buses);
	draw_event draw_event_inst(window, shaders, textures);
	post_processing_event post_processing_event_inst(
		window,
		shaders,
		textures,
		draw2d
	);
	post_render_pass_event post_render_event;

	key_controller keys(buses, {
		GLFW_KEY_W,
		GLFW_KEY_A,
		GLFW_KEY_S,
		GLFW_KEY_D,
		GLFW_KEY_LEFT_SHIFT,
		GLFW_KEY_F,
		GLFW_KEY_ESCAPE,
		GLFW_KEY_T,
		GLFW_KEY_P,
		GLFW_KEY_PERIOD
		});
	mouse_controller mouse(buses, {
		GLFW_MOUSE_BUTTON_LEFT,
		GLFW_MOUSE_BUTTON_RIGHT
		}, GLFW_KEY_ESCAPE);
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
		GLFW_KEY_P,
		GLFW_KEY_PERIOD
	);

	flashlight lc(buses, pl, w, GLFW_KEY_F);

	printf(help_text);

	while (! glfwWindowShouldClose(window)) {
		buses.render.fire(pre_render_event);
		buses.render.fire(draw_event_inst);
		buses.render.fire(post_processing_event_inst);

		glfwSwapBuffers(window);
		glfwPollEvents();

		buses.render.fire(post_render_event);
	}

	program_stop_event program_stop(0);
	buses.lifecycle.fire(program_stop);

	glfwTerminate();
	return 0;
}
