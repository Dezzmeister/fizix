#pragma once
#include <vector>
#include "particle_emitter.h"
#include "physics/particle.h"

struct particle {
	using clock = std::chrono::steady_clock;
	using time_point = std::chrono::time_point<clock>;
	using duration = std::chrono::milliseconds;

	phys::particle physics;
	time_point created_at;

	duration age(const time_point &now) const;
};

class physical_particle_emitter : public particle_emitter {
public:
	phys::vec3 pos;
	phys::vec3 base_vel;
	phys::real rand_pos_scale;
	phys::real rand_vel_scale;
	size_t new_particles_per_frame;
	phys::real particle_mass;
	phys::real damping;
	phys::vec3 gravity;
	particle::duration expiry_time;
	glm::vec4 start_color;
	glm::vec4 end_color;
	float max_particle_size;

	physical_particle_emitter(
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
	);

	void start() override;
	void stop() override;
	bool is_done() const override;

	void update(float millis) override;
	void prepare_draw(draw_event &event, const shader_program &shader) const override;
	void draw() const override;

	const std::string& shader_name() const override;

private:
	unique_handle<unsigned int> vao;
	unique_handle<unsigned int> pos_vbo;
	unique_handle<unsigned int> color_vbo;

	std::vector<particle> particles;
	std::vector<glm::vec3> pos_buf;
	std::vector<glm::vec4> color_buf;

	size_t max_particles;
	size_t starting_particles;

	phys::vec3 random_particle_pos() const;
	phys::vec3 random_particle_vel() const;
	int64_t next_dead_particle_pos(size_t start) const;

	bool is_particle_dead(const particle &p, const particle::time_point &now) const;
	glm::vec4 particle_color(const particle &p, const particle::time_point &now) const;
};