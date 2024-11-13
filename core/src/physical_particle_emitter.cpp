#include <random>
#include "gl.h"
#include "physical_particle_emitter.h"
#include "shader_constants.h"
#include "util.h"

using namespace phys::literals;
using namespace std::literals::chrono_literals;

namespace {
	std::random_device rng;
	std::mt19937 gen(rng());
	std::uniform_real_distribution<phys::real> distrib(-0.5_r, 0.5_r);

	const std::string emitter_shader_name = "particle_color";

	float lerp(float a, float b, float s) {
		return a + (b - a) * s;
	}
}

particle::duration particle::age(const particle::time_point &now) const {
	return std::chrono::duration_cast<particle::duration>(now - created_at);
}

physical_particle_emitter::physical_particle_emitter(
	phys::vec3 _pos,
	phys::vec3 _base_vel,
	phys::real _rand_pos_scale,
	phys::real _rand_vel_scale,
	size_t _max_particles,
	size_t _new_particles_per_frame,
	size_t _starting_particles,
	phys::real _particle_mass,
	phys::real _damping,
	phys::vec3 _gravity,
	particle::duration _expiry_time,
	glm::vec4 _start_color,
	glm::vec4 _end_color,
	float _max_particle_size
) :
	particle_emitter(),
	pos(_pos),
	base_vel(_base_vel),
	rand_pos_scale(_rand_pos_scale),
	rand_vel_scale(_rand_vel_scale),
	new_particles_per_frame(_new_particles_per_frame),
	particle_mass(_particle_mass),
	damping(_damping),
	gravity(_gravity),
	expiry_time(_expiry_time),
	start_color(_start_color),
	end_color(_end_color),
	max_particle_size(_max_particle_size),
	vao(0, [](unsigned int handle) {
	glDeleteVertexArrays(1, &handle);
		}),
	pos_vbo(0, [](unsigned int handle) {
	glDeleteBuffers(1, &handle);
		}),
	color_vbo(0, [](unsigned int handle) {
	glDeleteBuffers(1, &handle);
		}),
	particles{},
	pos_buf{},
	color_buf{},
	max_particles(_max_particles),
	starting_particles(_starting_particles)
{}

void physical_particle_emitter::update(float millis) {
	float dt = millis / 1000;
	const particle::time_point now = particle::clock::now();

	int64_t dead_particle_pos = 0;

	for (size_t i = 0; i < new_particles_per_frame; i++) {
		dead_particle_pos = next_dead_particle_pos((size_t)dead_particle_pos);

		if (dead_particle_pos == -1) {
			break;
		}

		particle &p = particles[dead_particle_pos];
		p.physics.pos = random_particle_pos();
		p.physics.vel = random_particle_vel();
		p.physics.acc = phys::vec3(0.0_r);
		p.physics.force = gravity;
		p.physics.damping = damping;
		p.physics.set_mass(particle_mass);
		p.created_at = now;
	}

	pos_buf.clear();
	color_buf.clear();

	for (size_t i = 0; i < particles.size(); i++) {
		particle &p = particles[i];
		p.physics.acc = phys::vec3(0.0_r);

		if (! is_particle_dead(p, now)) {
			p.physics.integrate(dt);
			pos_buf.push_back(phys::to_glm<glm::vec3>(p.physics.pos));
			color_buf.push_back(particle_color(p, now));
		}
	}

	assert(pos_buf.size() == color_buf.size());

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, pos_vbo);
	glBufferData(GL_ARRAY_BUFFER, pos_buf.size() * sizeof(glm::vec3), pos_buf.data(), GL_STREAM_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, color_vbo);
	glBufferData(GL_ARRAY_BUFFER, color_buf.size() * sizeof(glm::vec4), color_buf.data(), GL_STREAM_DRAW);
}

void physical_particle_emitter::prepare_draw(draw_event&, const shader_program &shader) const {
	static constexpr int max_particle_size_loc = util::find_in_map(constants::shader_locs, "max_particle_size");

	shader.set_uniform(max_particle_size_loc, max_particle_size);

	glBindVertexArray(vao);
}

void physical_particle_emitter::draw() const {
	glDrawArrays(GL_POINTS, 0, (GLsizei)pos_buf.size());
}

void physical_particle_emitter::start() {
	particle::time_point now = particle::clock::now();

	for (size_t i = 0; i < starting_particles; i++) {
		phys::particle physics{};
		physics.pos = random_particle_pos();
		physics.vel = random_particle_vel();
		physics.force = gravity;
		physics.set_mass(particle_mass);
		physics.damping = damping;

		particle p = {
			.physics = physics,
			.created_at = now
		};

		particles.push_back(p);

		pos_buf.push_back(phys::to_glm<glm::vec3>(physics.pos));
		color_buf.push_back(particle_color(p, now));
	}

	for (size_t i = starting_particles; i < max_particles; i++) {
		phys::particle physics{};
		physics.force = gravity;
		physics.set_mass(particle_mass);

		particle p = {
			.physics = physics,
			.created_at = particle::time_point(0ms)
		};

		particles.push_back(p);
	}

	assert(pos_buf.size() == color_buf.size());

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &pos_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, pos_vbo);
	glBufferData(GL_ARRAY_BUFFER, pos_buf.size() * sizeof(glm::vec3), pos_buf.data(), GL_STREAM_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &color_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, color_vbo);
	glBufferData(GL_ARRAY_BUFFER, color_buf.size() * sizeof(glm::vec4), color_buf.data(), GL_STREAM_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
}

bool physical_particle_emitter::is_done() const {
	const particle::time_point now = particle::clock::now();

	for (const particle &p : particles) {
		if (! is_particle_dead(p, now)) {
			return false;
		}
	}

	return true;
}

void physical_particle_emitter::stop() {
	for (particle &p : particles) {
		p.created_at = particle::time_point(0ms);
	}
}

const std::string& physical_particle_emitter::shader_name() const {
	return emitter_shader_name;
}

phys::vec3 physical_particle_emitter::random_particle_pos() const {
	phys::vec3 offset(
		distrib(gen),
		distrib(gen),
		distrib(gen)
	);

	return pos + (offset * rand_pos_scale);
}

phys::vec3 physical_particle_emitter::random_particle_vel() const {
	phys::vec3 offset(
		distrib(gen),
		distrib(gen),
		distrib(gen)
	);

	return base_vel + (offset * rand_vel_scale);
}

int64_t physical_particle_emitter::next_dead_particle_pos(size_t start) const {
	const particle::time_point now = particle::clock::now();

	for (size_t i = start; i < particles.size(); i++) {
		if (is_particle_dead(particles[i], now)) {
			return i;
		}
	}

	return -1;
}

glm::vec4 physical_particle_emitter::particle_color(const particle &p, const particle::time_point &now) const {
	long long curr_age = p.age(now).count();
	long long max_age = expiry_time.count();
	float age_f = ((float)curr_age) / max_age;

	if (age_f > 1.0) {
		return end_color;
	}

	return glm::vec4(
		lerp(start_color.r, end_color.r, age_f),
		lerp(start_color.g, end_color.g, age_f),
		lerp(start_color.b, end_color.b, age_f),
		lerp(start_color.a, end_color.a, age_f)
	);
}

bool physical_particle_emitter::is_particle_dead(const particle &p, const particle::time_point &now) const {
	return (now - p.created_at) > expiry_time;
}