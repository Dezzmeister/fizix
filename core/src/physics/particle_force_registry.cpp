#include "physics/particle_force_registry.h"

bool phys::operator==(const phys::particle_force_registration &a, const phys::particle_force_registration &b) {
	return (a.p == b.p && a.fg == b.fg);
}

void phys::particle_force_registry::add(phys::particle * p, phys::particle_force_generator * fg) {
	registrations.push_back({
		.p = p,
		.fg = fg
		});
}

void phys::particle_force_registry::remove(phys::particle * p, phys::particle_force_generator * fg) {
	std::erase(registrations, phys::particle_force_registration{
		.p = p,
		.fg = fg
		});
}

void phys::particle_force_registry::clear() {
	registrations.clear();
}

void phys::particle_force_registry::update_forces(phys::real duration) {
	for (auto &[p, fg] : registrations) {
		fg->update_force(*p, duration);
	}
}
