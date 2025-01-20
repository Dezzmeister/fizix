#define _USE_MATH_DEFINES
#include <math.h>
#include "logging.h"
#include "physics/collision/contact_generator.h"
#include "physics/collision/primitives.h"
#include "physics/collision/vclip.h"
#include "platform/platform.h"
#include "test.h"

using namespace test;
using namespace phys::literals;

namespace {
	const phys::contact_generator collider{};
	std::vector<phys::contact> contacts{};
	phys::rigid_body sphere_body_1{};
	phys::rigid_body sphere_body_2{};
	phys::rigid_body box_body_1{};
	phys::rigid_body box_body_2{};

	phys::box box_1(&box_body_1, phys::identity<phys::mat4>(), phys::vec3(1.0_r));
	phys::box box_2(&box_body_2, phys::identity<phys::mat4>(), phys::vec3(1.0_r));

	phys::quat make_rot(phys::real angle, const phys::vec3 &axis) {
		return phys::quat(std::cos(angle / 2.0_r), std::sin(angle / 2.0_r) * axis);
	}
}

void setup_collision_tests() {
	describe("Rigid body collisions", []() {
		describe("between a sphere and a sphere", []() {
			after_each([&]() {
				contacts.clear();
				sphere_body_1 = {};
				sphere_body_2 = {};
			});

			it("detects two interpenetrating spheres", [&]() {
				sphere_body_2.pos.y = 1.8_r;

				phys::sphere s1(&sphere_body_1, phys::identity<phys::mat4>(), 1.0_r);
				phys::sphere s2(&sphere_body_2, phys::identity<phys::mat4>(), 1.0_r);

				collider.generate_contacts(s1, s2, contacts);

				expect(contacts).to_have_size(1).annd()
					.to_have_item(phys::contact(
						&sphere_body_1,
						&sphere_body_2,
						phys::vec3(0.0_r, 0.9_r, 0.0_r),
						phys::vec3(0.0_r, 1.0_r, 0.0_r),
						0.2_r
					)).orr()
					.to_have_item(phys::contact(
						&sphere_body_2,
						&sphere_body_1,
						phys::vec3(0.0_r, 0.9_r, 0.0_r),
						phys::vec3(0.0_r, -1.0_r, 0.0_r),
						0.2_r
					));
			});

			it("detects two interpenetrating spheres when the arguments are swapped", [&]() {
				sphere_body_2.pos.y = 1.8_r;

				phys::sphere s1(&sphere_body_1, phys::identity<phys::mat4>(), 1.0_r);
				phys::sphere s2(&sphere_body_2, phys::identity<phys::mat4>(), 1.0_r);

				collider.generate_contacts(s2, s1, contacts);

				expect(contacts).to_have_size(1).annd()
					.to_have_item(phys::contact(
						&sphere_body_1,
						&sphere_body_2,
						phys::vec3(0.0_r, 0.9_r, 0.0_r),
						phys::vec3(0.0_r, 1.0_r, 0.0_r),
						0.2_r
					)).orr()
					.to_have_item(phys::contact(
						&sphere_body_2,
						&sphere_body_1,
						phys::vec3(0.0_r, 0.9_r, 0.0_r),
						phys::vec3(0.0_r, -1.0_r, 0.0_r),
						0.2_r
					));
			});

			it("does not generate a contact for two spheres that are not touching", [&]() {
				sphere_body_2.pos.y = 5.0_r;

				phys::sphere s1(&sphere_body_1, phys::identity<phys::mat4>(), 1.0_r);
				phys::sphere s2(&sphere_body_2, phys::identity<phys::mat4>(), 1.0_r);

				collider.generate_contacts(s1, s2, contacts);

				expect(contacts).to_have_size(0);
			});
		});

		describe("between a sphere and a plane", []() {
			after_each([&]() {
				contacts.clear();
				sphere_body_1 = {};
			});

			it("generates a contact for a sphere and a plane in the same direction", [&]() {
				sphere_body_1.pos.y = 5.0_r;

				phys::sphere s(&sphere_body_1, phys::identity<phys::mat4>(), 1.0_r);
				phys::plane p(nullptr, phys::vec3(0.0_r, 1.0_r, 0.0_r), 5.5_r);

				collider.generate_contacts(s, p, contacts);

				expect(contacts).to_have_size(1).annd()
					.to_have_item(phys::contact(
						&sphere_body_1,
						nullptr,
						phys::vec3(0.0_r, 5.5_r, 0.0_r),
						phys::vec3(0.0_r, 1.0_r, 0.0_r),
						0.5_r
					));
			});

			it("does not generate a contact for a sphere and a plane in the same direction", [&]() {
				sphere_body_1.pos.y = 5.0_r;

				phys::sphere s(&sphere_body_1, phys::identity<phys::mat4>(), 1.0_r);
				phys::plane p(nullptr, phys::vec3(0.0_r, 1.0_r, 0.0_r), 6.01_r);

				collider.generate_contacts(s, p, contacts);

				expect(contacts).to_have_size(0);
			});

			it("generates a contact for a sphere and a plane in opposite directions", [&]() {
				sphere_body_1.pos.y = 0.5_r;

				phys::sphere s(&sphere_body_1, phys::identity<phys::mat4>(), 1.0_r);
				phys::plane p(nullptr, phys::vec3(0.0_r, -1.0_r, 0.0_r), 0.4_r);

				collider.generate_contacts(s, p, contacts);

				expect(contacts).to_have_size(1).annd()
					.to_have_item(phys::contact(
						&sphere_body_1,
						nullptr,
						phys::vec3(0.0_r, -0.4_r, 0.0_r),
						phys::vec3(0.0_r, -1.0_r, 0.0_r),
						0.1_r
					));
			});

			it("does not generate a contact for a sphere and a plane in opposite directions", [&]() {
				sphere_body_1.pos.y = 0.5_r;

				phys::sphere s(&sphere_body_1, phys::identity<phys::mat4>(), 1.0_r);
				phys::plane p(nullptr, phys::vec3(0.0_r, -1.0_r, 0.0_r), 1.0_r);

				collider.generate_contacts(s, p, contacts);

				expect(contacts).to_have_size(0);
			});

			it("generates a contact for a plane perpendicular to the sphere's pos (+z)", [&]() {
				sphere_body_1.pos.y = 5.0_r;

				phys::sphere s(&sphere_body_1, phys::identity<phys::mat4>(), 1.0_r);
				phys::plane p(nullptr, phys::vec3(0.0_r, 0.0_r, 1.0_r), 0.5_r);

				collider.generate_contacts(s, p, contacts);

				expect(contacts).to_have_size(1).annd()
					.to_have_item(phys::contact(
						&sphere_body_1,
						nullptr,
						phys::vec3(0.0_r, 5.0_r, 0.5_r),
						phys::vec3(0.0_r, 0.0_r, 1.0_r),
						0.5_r
					));
			});

			it("generates a contact for a plane perpendicular to the sphere's pos (-z)", [&]() {
				sphere_body_1.pos.y = 5.0_r;

				phys::sphere s(&sphere_body_1, phys::identity<phys::mat4>(), 1.0_r);
				phys::plane p(nullptr, phys::vec3(0.0_r, 0.0_r, -1.0_r), 0.5_r);

				collider.generate_contacts(s, p, contacts);

				expect(contacts).to_have_size(1).annd()
					.to_have_item(phys::contact(
						&sphere_body_1,
						nullptr,
						phys::vec3(0.0_r, 5.0_r, -0.5_r),
						phys::vec3(0.0_r, 0.0_r, -1.0_r),
						0.5_r
					));
			});

			it("does not generate a contact for a plane perpendicular to the sphere's pos", [&]() {
				sphere_body_1.pos.y = 5.0_r;

				phys::sphere s(&sphere_body_1, phys::identity<phys::mat4>(), 1.0_r);
				phys::plane p(nullptr, phys::vec3(0.0_r, 0.0_r, 1.0_r), 1.1_r);

				collider.generate_contacts(s, p, contacts);

				expect(contacts).to_have_size(0);
			});

			it("generates a contact when the arguments are swapped", [&]() {
				sphere_body_1.pos.y = 5.0_r;

				phys::sphere s(&sphere_body_1, phys::identity<phys::mat4>(), 1.0_r);
				phys::plane p(nullptr, phys::vec3(0.0_r, 0.0_r, -1.0_r), 0.5_r);

				collider.generate_contacts(p, s, contacts);

				expect(contacts).to_have_size(1).annd()
					.to_have_item(phys::contact(
						&sphere_body_1,
						nullptr,
						phys::vec3(0.0_r, 5.0_r, -0.5_r),
						phys::vec3(0.0_r, 0.0_r, -1.0_r),
						0.5_r
					));
			});
		});

		describe("between a plane and a box", []() {
			describe("with no offset", []() {
				after_each([&]() {
					contacts.clear();
					box_body_1 = {};
				});

				it("generates contacts for a box with four vertices below the plane", [&]() {
					box_body_1.pos.y = 1.0_r;
					box_body_1.calculate_derived_data();

					phys::plane p(nullptr, phys::vec3(0.0_r, 1.0_r, 0.0_r), 0.1_r);
					phys::box b(&box_body_1, phys::identity<phys::mat4>(), phys::vec3(1.0_r));

					collider.generate_contacts(p, b, contacts);

					expect(contacts).to_have_size(4).annd()
						.to_have_item(phys::contact(
							&box_body_1,
							nullptr,
							phys::vec3(1.0_r, 0.0_r, 1.0_r),
							phys::vec3(0.0_r, -1.0_r, 0.0_r),
							0.1_r
						)).annd()
						.to_have_item(phys::contact(
							&box_body_1,
							nullptr,
							phys::vec3(-1.0_r, 0.0_r, 1.0_r),
							phys::vec3(0.0_r, -1.0_r, 0.0_r),
							0.1_r
						)).annd()
						.to_have_item(phys::contact(
							&box_body_1,
							nullptr,
							phys::vec3(1.0_r, 0.0_r, -1.0_r),
							phys::vec3(0.0_r, -1.0_r, 0.0_r),
							0.1_r
						)).annd()
						.to_have_item(phys::contact(
							&box_body_1,
							nullptr,
							phys::vec3(-1.0_r, 0.0_r, -1.0_r),
							phys::vec3(0.0_r, -1.0_r, 0.0_r),
							0.1_r
						));
				});

				it("generates contacts for a box with two vertices below the plane", [&]() {
					constexpr phys::real angle = (phys::real)M_PI / 4.0_r;
					phys::vec3 axis(0.0_r, 0.0_r, 1.0_r);

					box_body_1.pos.y = 1.0_r;
					box_body_1.rot = phys::quat(std::cos(angle / 2.0_r), std::sin(angle / 2.0_r) * axis);
					box_body_1.calculate_derived_data();

					phys::plane p(nullptr, phys::vec3(0.0_r, 1.0_r, 0.0_r), 0.0_r);
					phys::box b(&box_body_1, phys::identity<phys::mat4>(), phys::vec3(1.0_r));

					collider.generate_contacts(p, b, contacts);

					expect(contacts).to_have_size(2).annd()
						.to_have_item(phys::contact(
							&box_body_1,
							nullptr,
							phys::vec3(0.0_r, 1.0_r - std::sqrt(2.0_r), -1.0_r),
							phys::vec3(0.0_r, -1.0_r, 0.0_r),
							std::sqrt(2.0_r) - 1.0_r
						)).annd()
						.to_have_item(phys::contact(
							&box_body_1,
							nullptr,
							phys::vec3(0.0_r, 1.0_r - std::sqrt(2.0_r), 1.0_r),
							phys::vec3(0.0_r, -1.0_r, 0.0_r),
							std::sqrt(2.0_r) - 1.0_r
						));
				});

				it("generates contacts for a box with two vertices below the plane and the plane facing -y", [&]() {
					constexpr phys::real angle = (phys::real)M_PI / 4.0_r;
					phys::vec3 axis(0.0_r, 0.0_r, 1.0_r);

					box_body_1.pos.y = 1.0_r;
					box_body_1.rot = phys::quat(std::cos(angle / 2.0_r), std::sin(angle / 2.0_r) * axis);
					box_body_1.calculate_derived_data();

					phys::plane p(nullptr, phys::vec3(0.0_r, -1.0_r, 0.0_r), 0.0_r);
					phys::box b(&box_body_1, phys::identity<phys::mat4>(), phys::vec3(1.0_r));

					collider.generate_contacts(p, b, contacts);

					expect(contacts).to_have_size(2).annd()
						.to_have_item(phys::contact(
							&box_body_1,
							nullptr,
							phys::vec3(0.0_r, 1.0_r - std::sqrt(2.0_r), -1.0_r),
							phys::vec3(0.0_r, -1.0_r, 0.0_r),
							std::sqrt(2.0_r) - 1.0_r
						)).annd()
						.to_have_item(phys::contact(
							&box_body_1,
							nullptr,
							phys::vec3(0.0_r, 1.0_r - std::sqrt(2.0_r), 1.0_r),
							phys::vec3(0.0_r, -1.0_r, 0.0_r),
							std::sqrt(2.0_r) - 1.0_r
						));
				});

				it("generates a contact for a box with one vertex below the plane", [&]() {
					constexpr phys::real half_z_angle = (phys::real)M_PI / 8.0_r;
					phys::real half_x_angle = std::asin(1.0_r / std::sqrt(3.0_r)) / 2.0_r;
					phys::quat z_rot(std::cos(half_z_angle), std::sin(half_z_angle) * phys::vec3(0.0_r, 0.0_r, 1.0_r));
					phys::quat x_rot(std::cos(half_x_angle), std::sin(half_x_angle) * phys::vec3(1.0_r, 0.0_r, 0.0_r));

					box_body_1.pos.y = 1.0_r;
					box_body_1.rot = x_rot * z_rot;
					box_body_1.calculate_derived_data();

					phys::plane p(nullptr, phys::vec3(0.0_r, 1.0_r, 0.0_r), 0.0_r);
					phys::box b(&box_body_1, phys::identity<phys::mat4>(), phys::vec3(1.0_r));

					collider.generate_contacts(p, b, contacts);

					expect(contacts).to_have_size(1).annd()
						.to_have_item(phys::contact(
							&box_body_1,
							nullptr,
							phys::vec3(0.0_r, 1.0_r - std::sqrt(3.0_r), 0.0_r),
							phys::vec3(0.0_r, -1.0_r, 0.0_r),
							std::sqrt(3.0_r) - 1.0_r
						));
				});

				it("generates contacts for a box with two vertices above the plane", [&]() {
					constexpr phys::real angle = (phys::real)M_PI / 4.0_r;
					phys::vec3 axis(0.0_r, 0.0_r, 1.0_r);

					box_body_1.pos.y = 1.0_r;
					box_body_1.rot = phys::quat(std::cos(angle / 2.0_r), std::sin(angle / 2.0_r) * axis);
					box_body_1.calculate_derived_data();

					phys::plane p(nullptr, phys::vec3(0.0_r, 1.0_r, 0.0_r), 2.0_r);
					phys::box b(&box_body_1, phys::identity<phys::mat4>(), phys::vec3(1.0_r));

					collider.generate_contacts(p, b, contacts);

					expect(contacts).to_have_size(2).annd()
						.to_have_item(phys::contact(
							&box_body_1,
							nullptr,
							phys::vec3(0.0_r, 1.0_r + std::sqrt(2.0_r), -1.0_r),
							phys::vec3(0.0_r, 1.0_r, 0.0_r),
							std::sqrt(2.0_r) - 1.0_r
						)).annd()
						.to_have_item(phys::contact(
							&box_body_1,
							nullptr,
							phys::vec3(0.0_r, 1.0_r + std::sqrt(2.0_r), 1.0_r),
							phys::vec3(0.0_r, 1.0_r, 0.0_r),
							std::sqrt(2.0_r) - 1.0_r
						));
				});

				it("generates contacts for a box with two vertices above the plane and the plane facing -y", [&]() {
					constexpr phys::real angle = (phys::real)M_PI / 4.0_r;
					phys::vec3 axis(0.0_r, 0.0_r, 1.0_r);

					box_body_1.pos.y = 1.0_r;
					box_body_1.rot = phys::quat(std::cos(angle / 2.0_r), std::sin(angle / 2.0_r) * axis);
					box_body_1.calculate_derived_data();

					phys::plane p(nullptr, phys::vec3(0.0_r, -1.0_r, 0.0_r), -2.0_r);
					phys::box b(&box_body_1, phys::identity<phys::mat4>(), phys::vec3(1.0_r));

					collider.generate_contacts(p, b, contacts);

					expect(contacts).to_have_size(2).annd()
						.to_have_item(phys::contact(
							&box_body_1,
							nullptr,
							phys::vec3(0.0_r, 1.0_r + std::sqrt(2.0_r), -1.0_r),
							phys::vec3(0.0_r, 1.0_r, 0.0_r),
							std::sqrt(2.0_r) - 1.0_r
						)).annd()
						.to_have_item(phys::contact(
							&box_body_1,
							nullptr,
							phys::vec3(0.0_r, 1.0_r + std::sqrt(2.0_r), 1.0_r),
							phys::vec3(0.0_r, 1.0_r, 0.0_r),
							std::sqrt(2.0_r) - 1.0_r
						));
				});

				it("generates contacts for a box below y = 0 with two vertices above the plane", [&]() {
					constexpr phys::real angle = (phys::real)M_PI / 4.0_r;
					phys::vec3 axis(0.0_r, 0.0_r, 1.0_r);

					box_body_1.pos.y = -1.0_r;
					box_body_1.rot = phys::quat(std::cos(angle / 2.0_r), std::sin(angle / 2.0_r) * axis);
					box_body_1.calculate_derived_data();

					phys::plane p(nullptr, phys::vec3(0.0_r, 1.0_r, 0.0_r), 0.0_r);
					phys::box b(&box_body_1, phys::identity<phys::mat4>(), phys::vec3(1.0_r));

					collider.generate_contacts(p, b, contacts);

					expect(contacts).to_have_size(2).annd()
						.to_have_item(phys::contact(
							&box_body_1,
							nullptr,
							phys::vec3(0.0_r, -1.0_r + std::sqrt(2.0_r), -1.0_r),
							phys::vec3(0.0_r, 1.0_r, 0.0_r),
							std::sqrt(2.0_r) - 1.0_r
						)).annd()
						.to_have_item(phys::contact(
							&box_body_1,
							nullptr,
							phys::vec3(0.0_r, -1.0_r + std::sqrt(2.0_r), 1.0_r),
							phys::vec3(0.0_r, 1.0_r, 0.0_r),
							std::sqrt(2.0_r) - 1.0_r
						));
				});

				it("generates contacts for a box below y = 0 with two vertices above the plane and the plane facing -y", [&]() {
					constexpr phys::real angle = (phys::real)M_PI / 4.0_r;
					phys::vec3 axis(0.0_r, 0.0_r, 1.0_r);

					box_body_1.pos.y = -1.0_r;
					box_body_1.rot = phys::quat(std::cos(angle / 2.0_r), std::sin(angle / 2.0_r) * axis);
					box_body_1.calculate_derived_data();

					phys::plane p(nullptr, phys::vec3(0.0_r, -1.0_r, 0.0_r), 0.0_r);
					phys::box b(&box_body_1, phys::identity<phys::mat4>(), phys::vec3(1.0_r));

					collider.generate_contacts(p, b, contacts);

					expect(contacts).to_have_size(2).annd()
						.to_have_item(phys::contact(
							&box_body_1,
							nullptr,
							phys::vec3(0.0_r, -1.0_r + std::sqrt(2.0_r), -1.0_r),
							phys::vec3(0.0_r, 1.0_r, 0.0_r),
							std::sqrt(2.0_r) - 1.0_r
						)).annd()
						.to_have_item(phys::contact(
							&box_body_1,
							nullptr,
							phys::vec3(0.0_r, -1.0_r + std::sqrt(2.0_r), 1.0_r),
							phys::vec3(0.0_r, 1.0_r, 0.0_r),
							std::sqrt(2.0_r) - 1.0_r
						));
				});

				it("generates contacts for a box far below y = 0 with two vertices above the plane", [&]() {
					constexpr phys::real angle = (phys::real)M_PI / 4.0_r;
					phys::vec3 axis(0.0_r, 0.0_r, 1.0_r);

					box_body_1.pos.y = -2.0_r;
					box_body_1.rot = phys::quat(std::cos(angle / 2.0_r), std::sin(angle / 2.0_r) * axis);
					box_body_1.calculate_derived_data();

					phys::plane p(nullptr, phys::vec3(0.0_r, -1.0_r, 0.0_r), 1.0_r);
					phys::box b(&box_body_1, phys::identity<phys::mat4>(), phys::vec3(1.0_r));

					collider.generate_contacts(p, b, contacts);

					expect(contacts).to_have_size(2).annd()
						.to_have_item(phys::contact(
							&box_body_1,
							nullptr,
							phys::vec3(0.0_r, -2.0_r + std::sqrt(2.0_r), -1.0_r),
							phys::vec3(0.0_r, 1.0_r, 0.0_r),
							std::sqrt(2.0_r) - 1.0_r
						)).annd()
						.to_have_item(phys::contact(
							&box_body_1,
							nullptr,
							phys::vec3(0.0_r, -2.0_r + std::sqrt(2.0_r), 1.0_r),
							phys::vec3(0.0_r, 1.0_r, 0.0_r),
							std::sqrt(2.0_r) - 1.0_r
						));
				});

				it("does not generate contacts for a box far below the plane", [&]() {
					box_body_1.pos.y = 1.0_r;
					box_body_1.calculate_derived_data();

					phys::plane p(nullptr, phys::vec3(0.0_r, 1.0_r, 0.0_r), 5.0_r);
					phys::box b(&box_body_1, phys::identity<phys::mat4>(), phys::vec3(1.0_r));

					collider.generate_contacts(p, b, contacts);

					expect(contacts).to_have_size(0);
				});

				it("generates contacts when the arguments are swapped", [&]() {
					constexpr phys::real angle = (phys::real)M_PI / 4.0_r;
					phys::vec3 axis(0.0_r, 0.0_r, 1.0_r);

					box_body_1.pos.y = -2.0_r;
					box_body_1.rot = phys::quat(std::cos(angle / 2.0_r), std::sin(angle / 2.0_r) * axis);
					box_body_1.calculate_derived_data();

					phys::plane p(nullptr, phys::vec3(0.0_r, -1.0_r, 0.0_r), 1.0_r);
					phys::box b(&box_body_1, phys::identity<phys::mat4>(), phys::vec3(1.0_r));

					collider.generate_contacts(b, p, contacts);

					expect(contacts).to_have_size(2).annd()
						.to_have_item(phys::contact(
							&box_body_1,
							nullptr,
							phys::vec3(0.0_r, -2.0_r + std::sqrt(2.0_r), -1.0_r),
							phys::vec3(0.0_r, 1.0_r, 0.0_r),
							std::sqrt(2.0_r) - 1.0_r
						)).annd()
						.to_have_item(phys::contact(
							&box_body_1,
							nullptr,
							phys::vec3(0.0_r, -2.0_r + std::sqrt(2.0_r), 1.0_r),
							phys::vec3(0.0_r, 1.0_r, 0.0_r),
							std::sqrt(2.0_r) - 1.0_r
						));
				});
			});

			describe("with an offset", []() {
				after_each([&]() {
					contacts.clear();
					box_body_1 = {};
				});

				it("generates contacts for a box offset by a translation", [&]() {
					box_body_1.pos.y = 2.0_r;
					box_body_1.calculate_derived_data();

					phys::plane p(nullptr, phys::vec3(0.0_r, 1.0_r, 0.0_r), 0.1_r);
					phys::box b(&box_body_1, phys::translate(phys::vec3(0.0_r, -1.0_r, 0.0_r)), phys::vec3(1.0_r));

					collider.generate_contacts(p, b, contacts);

					expect(contacts).to_have_size(4).annd()
						.to_have_item(phys::contact(
							&box_body_1,
							nullptr,
							phys::vec3(1.0_r, 0.0_r, 1.0_r),
							phys::vec3(0.0_r, -1.0_r, 0.0_r),
							0.1_r
						)).annd()
						.to_have_item(phys::contact(
							&box_body_1,
							nullptr,
							phys::vec3(-1.0_r, 0.0_r, 1.0_r),
							phys::vec3(0.0_r, -1.0_r, 0.0_r),
							0.1_r
						)).annd()
						.to_have_item(phys::contact(
							&box_body_1,
							nullptr,
							phys::vec3(1.0_r, 0.0_r, -1.0_r),
							phys::vec3(0.0_r, -1.0_r, 0.0_r),
							0.1_r
						)).annd()
						.to_have_item(phys::contact(
							&box_body_1,
							nullptr,
							phys::vec3(-1.0_r, 0.0_r, -1.0_r),
							phys::vec3(0.0_r, -1.0_r, 0.0_r),
							0.1_r
						));
				});

				it("generates contacts for a box offset by a rotation", [&]() {
					constexpr phys::real angle = (phys::real)M_PI / 4.0_r;
					phys::vec3 axis(0.0_r, 0.0_r, 1.0_r);

					// The box is offset from the rigid body by a rotation, but
					// the rigid body is translated in the +y direction
					box_body_1.pos.y = 1.0_r;
					phys::quat rot = phys::quat(std::cos(angle / 2.0_r), std::sin(angle / 2.0_r) * axis);
					box_body_1.calculate_derived_data();

					phys::plane p(nullptr, phys::vec3(0.0_r, 1.0_r, 0.0_r), 0.0_r);
					phys::box b(&box_body_1, phys::quat_to_mat4(rot), phys::vec3(1.0_r));

					collider.generate_contacts(p, b, contacts);

					expect(contacts).to_have_size(2).annd()
						.to_have_item(phys::contact(
							&box_body_1,
							nullptr,
							phys::vec3(0.0_r, 1.0_r - std::sqrt(2.0_r), -1.0_r),
							phys::vec3(0.0_r, -1.0_r, 0.0_r),
							std::sqrt(2.0_r) - 1.0_r
						)).annd()
						.to_have_item(phys::contact(
							&box_body_1,
							nullptr,
							phys::vec3(0.0_r, 1.0_r - std::sqrt(2.0_r), 1.0_r),
							phys::vec3(0.0_r, -1.0_r, 0.0_r),
							std::sqrt(2.0_r) - 1.0_r
						));
				});

				it("generates contacts for a box offset by a translation and a rotation", [&]() {
					constexpr phys::real angle = (phys::real)M_PI / 4.0_r;
					phys::vec3 axis(0.0_r, 0.0_r, 1.0_r);

					box_body_1.pos.y = 0.5_r;
					phys::quat rot(std::cos(angle / 2.0_r), std::sin(angle / 2.0_r) * axis);
					box_body_1.calculate_derived_data();

					phys::plane p(nullptr, phys::vec3(0.0_r, 1.0_r, 0.0_r), 0.0_r);
					phys::box b(&box_body_1, phys::translate(phys::vec3(0.0_r, 0.5_r, 0.0_r)) * phys::quat_to_mat4(rot), phys::vec3(1.0_r));

					collider.generate_contacts(p, b, contacts);

					expect(contacts).to_have_size(2).annd()
						.to_have_item(phys::contact(
							&box_body_1,
							nullptr,
							phys::vec3(0.0_r, 1.0_r - std::sqrt(2.0_r), -1.0_r),
							phys::vec3(0.0_r, -1.0_r, 0.0_r),
							std::sqrt(2.0_r) - 1.0_r
						)).annd()
						.to_have_item(phys::contact(
							&box_body_1,
							nullptr,
							phys::vec3(0.0_r, 1.0_r - std::sqrt(2.0_r), 1.0_r),
							phys::vec3(0.0_r, -1.0_r, 0.0_r),
							std::sqrt(2.0_r) - 1.0_r
						));
				});
			});
		});

		describe("between a sphere and a box", []() {
			describe("with no offset", []() {
				after_each([&]() {
					contacts.clear();
					sphere_body_1 = {};
					box_body_1 = {};
				});

				it("generates a contact for a sphere touching the face of a box and within the box's shadow", [&]() {
					sphere_body_1.pos.y = 1.8_r;

					phys::sphere s(&sphere_body_1, phys::identity<phys::mat4>(), 1.0_r);
					phys::box b(&box_body_1, phys::identity<phys::mat4>(), phys::vec3(2.0_r, 1.0_r, 2.0_r));

					collider.generate_contacts(s, b, contacts);

					expect(contacts).to_have_size(1).annd()
						.to_have_item(phys::contact(
							&sphere_body_1,
							&box_body_1,
							phys::vec3(0.0_r, 1.0_r, 0.0_r),
							phys::vec3(0.0_r, -1.0_r, 0.0_r),
							0.2_r
						));
				});

				it("generates a contact for a sphere touching the face of a box and partially outside of the box's shadow", [&]() {
					sphere_body_1.pos = phys::vec3(1.8_r, 1.8_r, 0.0_r);

					phys::sphere s(&sphere_body_1, phys::identity<phys::mat4>(), 1.0_r);
					phys::box b(&box_body_1, phys::identity<phys::mat4>(), phys::vec3(2.0_r, 1.0_r, 2.0_r));

					collider.generate_contacts(s, b, contacts);

					expect(contacts).to_have_size(1).annd()
						.to_have_item(phys::contact(
							&sphere_body_1,
							&box_body_1,
							phys::vec3(1.8_r, 1.0_r, 0.0_r),
							phys::vec3(0.0_r, -1.0_r, 0.0_r),
							0.2_r
						));
				});

				it("generates a contact for a sphere touching the edge of a box", [&]() {
					sphere_body_1.pos = phys::vec3(2.0_r, 2.0_r, 0.0_r);

					phys::sphere s(&sphere_body_1, phys::identity<phys::mat4>(), 1.8_r);
					phys::box b(&box_body_1, phys::identity<phys::mat4>(), phys::vec3(1.0_r));

					collider.generate_contacts(s, b, contacts);

					expect(contacts).to_have_size(1).annd()
						.to_have_item(phys::contact(
							&sphere_body_1,
							&box_body_1,
							phys::vec3(1.0_r, 1.0_r, 0.0_r),
							phys::normalize(phys::vec3(-1.0_r, -1.0_r, 0.0_r)),
							1.8_r - std::sqrt(2.0_r)
						));
				});

				it("generates a contact for a sphere touching the corner of a box", [&]() {
					sphere_body_1.pos = phys::vec3(2.0_r);

					phys::sphere s(&sphere_body_1, phys::identity<phys::mat4>(), 1.8_r);
					phys::box b(&box_body_1, phys::identity<phys::mat4>(), phys::vec3(1.0_r));

					collider.generate_contacts(s, b, contacts);

					expect(contacts).to_have_size(1).annd()
						.to_have_item(phys::contact(
							&sphere_body_1,
							&box_body_1,
							phys::vec3(1.0_r),
							phys::normalize(phys::vec3(-1.0_r)),
							1.8_r - std::sqrt(3.0_r)
						));
				});

				it("generates a contact for a sphere touching a translated box", [&]() {
					sphere_body_1.pos = phys::vec3(10.0_r, 1.8_r, 0.0_r);
					box_body_1.pos.x = 10.0_r;

					sphere_body_1.calculate_derived_data();
					box_body_1.calculate_derived_data();

					phys::sphere s(&sphere_body_1, phys::identity<phys::mat4>(), 1.0_r);
					phys::box b(&box_body_1, phys::identity<phys::mat4>(), phys::vec3(2.0_r, 1.0_r, 2.0_r));

					collider.generate_contacts(s, b, contacts);

					expect(contacts).to_have_size(1).annd()
						.to_have_item(phys::contact(
							&sphere_body_1,
							&box_body_1,
							phys::vec3(10.0_r, 1.0_r, 0.0_r),
							phys::vec3(0.0_r, -1.0_r, 0.0_r),
							0.2_r
						));
				});

				it("generates a contact for a sphere touching a rotated box", [&]() {
					constexpr phys::real angle = (phys::real)M_PI / 4.0_r;
					phys::vec3 axis(1.0_r, 0.0_r, 0.0_r);

					box_body_1.rot = phys::quat(std::cos(angle / 2.0_r), std::sin(angle / 2.0_r) * axis);
					sphere_body_1.pos = phys::vec3(0.0_r, 1.5_r, 1.5_r);

					sphere_body_1.calculate_derived_data();
					box_body_1.calculate_derived_data();

					phys::sphere s(&sphere_body_1, phys::identity<phys::mat4>(), 1.5_r);
					phys::box b(&box_body_1, phys::identity<phys::mat4>(), phys::vec3(1.0_r));

					collider.generate_contacts(s, b, contacts);

					expect(contacts).to_have_size(1).annd()
						.to_have_item(phys::contact(
							&sphere_body_1,
							&box_body_1,
							phys::vec3(0.0_r, std::sqrt(2.0_r) / 2.0_r, std::sqrt(2.0_r) / 2.0_r),
							phys::normalize(phys::vec3(0.0_r, -1.0_r, -1.0_r)),
							1.0_r + 1.5_r * (1.0_r - std::sqrt(2.0_r))
						));
				});

				it("generates a contact when the arguments are swapped", [&]() {
					sphere_body_1.pos.y = 1.8_r;

					phys::sphere s(&sphere_body_1, phys::identity<phys::mat4>(), 1.0_r);
					phys::box b(&box_body_1, phys::identity<phys::mat4>(), phys::vec3(2.0_r, 1.0_r, 2.0_r));

					collider.generate_contacts(b, s, contacts);

					expect(contacts).to_have_size(1).annd()
						.to_have_item(phys::contact(
							&sphere_body_1,
							&box_body_1,
							phys::vec3(0.0_r, 1.0_r, 0.0_r),
							phys::vec3(0.0_r, -1.0_r, 0.0_r),
							0.2_r
						));
				});

				it("does not generate a contact for a sphere far from a box", [&]() {
					sphere_body_1.pos.y = 10.0_r;

					sphere_body_1.calculate_derived_data();

					phys::sphere s(&sphere_body_1, phys::identity<phys::mat4>(), 1.0_r);
					phys::box b(&box_body_1, phys::identity<phys::mat4>(), phys::vec3(1.0_r));

					collider.generate_contacts(s, b, contacts);

					expect(contacts).to_have_size(0);
				});
			});

			describe("with an offset", []() {
				after_each([&]() {
					contacts.clear();
					sphere_body_1 = {};
					box_body_1 = {};
				});

				it("generates a contact for a box offset by a translation", [&]() {
					sphere_body_1.pos = phys::vec3(10.0_r, 1.8_r, 0.0_r);

					sphere_body_1.calculate_derived_data();

					phys::sphere s(&sphere_body_1, phys::identity<phys::mat4>(), 1.0_r);
					phys::box b(
						&box_body_1,
						phys::translate(phys::vec3(10.0_r, 0.0_r, 0.0_r)),
						phys::vec3(2.0_r, 1.0_r, 2.0_r)
					);

					collider.generate_contacts(s, b, contacts);

					expect(contacts).to_have_size(1).annd()
						.to_have_item(phys::contact(
							&sphere_body_1,
							&box_body_1,
							phys::vec3(10.0_r, 1.0_r, 0.0_r),
							phys::vec3(0.0_r, -1.0_r, 0.0_r),
							0.2_r
						));
				});

				it("generates a contact for a box offset by a rotation", [&]() {
					constexpr phys::real angle = (phys::real)M_PI / 4.0_r;
					phys::vec3 axis(1.0_r, 0.0_r, 0.0_r);

					phys::quat rot(std::cos(angle / 2.0_r), std::sin(angle / 2.0_r) * axis);
					sphere_body_1.pos = phys::vec3(0.0_r, 1.5_r, 1.5_r);

					sphere_body_1.calculate_derived_data();
					box_body_1.calculate_derived_data();

					phys::sphere s(&sphere_body_1, phys::identity<phys::mat4>(), 1.5_r);
					phys::box b(&box_body_1, phys::quat_to_mat4(rot), phys::vec3(1.0_r));

					collider.generate_contacts(s, b, contacts);

					expect(contacts).to_have_size(1).annd()
						.to_have_item(phys::contact(
							&sphere_body_1,
							&box_body_1,
							phys::vec3(0.0_r, std::sqrt(2.0_r) / 2.0_r, std::sqrt(2.0_r) / 2.0_r),
							phys::normalize(phys::vec3(0.0_r, -1.0_r, -1.0_r)),
							1.0_r + 1.5_r * (1.0_r - std::sqrt(2.0_r))
						));
				});

				it("generates a contact for a box offset by a translation and a rotation", [&]() {
					constexpr phys::real angle = (phys::real)M_PI / 4.0_r;
					phys::vec3 axis(1.0_r, 0.0_r, 0.0_r);

					phys::quat rot(std::cos(angle / 2.0_r), std::sin(angle / 2.0_r) * axis);
					sphere_body_1.pos = phys::vec3(10.0_r, 1.5_r, 1.5_r);

					sphere_body_1.calculate_derived_data();
					box_body_1.calculate_derived_data();

					phys::sphere s(&sphere_body_1, phys::identity<phys::mat4>(), 1.5_r);
					phys::box b(
						&box_body_1,
						phys::translate(phys::vec3(10.0_r, 0.0_r, 0.0_r)) * phys::quat_to_mat4(rot),
						phys::vec3(1.0_r)
					);

					collider.generate_contacts(s, b, contacts);

					expect(contacts).to_have_size(1).annd()
						.to_have_item(phys::contact(
							&sphere_body_1,
							&box_body_1,
							phys::vec3(10.0_r, std::sqrt(2.0_r) / 2.0_r, std::sqrt(2.0_r) / 2.0_r),
							phys::normalize(phys::vec3(0.0_r, -1.0_r, -1.0_r)),
							1.0_r + 1.5_r * (1.0_r - std::sqrt(2.0_r))
						));
				});

				it("generates a contact for a box offset by a translation and a rotation after a translation", [&]() {
					constexpr phys::real angle = (phys::real)M_PI / 4.0_r;
					phys::vec3 axis(1.0_r, 0.0_r, 0.0_r);

					phys::quat rot(std::cos(angle / 2.0_r), std::sin(angle / 2.0_r) * axis);
					sphere_body_1.pos = phys::vec3(5.0_r, 6.5_r, 1.5_r);
					box_body_1.pos.y = 5.0_r;

					sphere_body_1.calculate_derived_data();
					box_body_1.calculate_derived_data();

					phys::sphere s(&sphere_body_1, phys::identity<phys::mat4>(), 1.5_r);
					phys::box b(
						&box_body_1,
						phys::translate(phys::vec3(5.0_r, 0.0_r, 0.0_r)) * phys::quat_to_mat4(rot),
						phys::vec3(1.0_r)
					);

					collider.generate_contacts(s, b, contacts);

					expect(contacts).to_have_size(1).annd()
						.to_have_item(phys::contact(
							&sphere_body_1,
							&box_body_1,
							phys::vec3(5.0_r, 5.0_r + std::sqrt(2.0_r) / 2.0_r, std::sqrt(2.0_r) / 2.0_r),
							phys::normalize(phys::vec3(0.0_r, -1.0_r, -1.0_r)),
							1.0_r + 1.5_r * (1.0_r - std::sqrt(2.0_r))
						));
				});
			});
		});

		describe("VClip", []() {
			describe("auxiliary", []() {
				it("compares edges when the vertices are swapped", []() {
					phys::vclip::edge e(4, 9);

					expect(e).to_be(phys::vclip::edge(9, 4));
				});

				it("compares faces when the vertices are rotated", []() {
					phys::vclip::face f({ 1, 2, 3, 4 });

					expect(f).to_be(phys::vclip::face({ 2, 3, 4, 1 })).annd()
						.to_be(phys::vclip::face({ 3, 4, 1, 2 })).annd()
						.to_be(phys::vclip::face({ 4, 1, 2, 3 })).annd()
						.naht().to_be(phys::vclip::face({ 1, 3, 2, 4 }));
				});

				it("fails to validate a polyhedron with a missing face (an open polyhedron)", []() {
					// A pyramid missing the bottom face
					phys::vclip::polyhedron p{
						.vertices = {
							phys::vclip::vertex(phys::vec3(1.0_r, 0.0_r, 0.0_r), 0),
							phys::vclip::vertex(phys::vec3(0.0_r, 0.0_r, 1.0_r), 1),
							phys::vclip::vertex(phys::vec3(0.0_r, 0.0_r, -1.0_r), 2),
							phys::vclip::vertex(phys::vec3(0.0_r, 1.0_r, 0.0_r), 3)
						},
						.edges = {
							phys::vclip::edge(0, 1),
							phys::vclip::edge(1, 2),
							phys::vclip::edge(2, 0),
							phys::vclip::edge(0, 3),
							phys::vclip::edge(1, 3),
							phys::vclip::edge(2, 3)
						},
						.faces = {
							phys::vclip::face({ 0, 3, 1 }),
							phys::vclip::face({ 1, 3, 2 }),
							phys::vclip::face({ 2, 3, 0 }),
							// (0, 1, 2) is missing
						}
					};

					expect(p.euler_characteristic()).naht().to_be(2);
					try {
						// TODO: Callable matcher with `to_throw`
						p.validate();
						fail("Expected polyhedron validation to fail");
					} catch (const phys::vclip::geometry_error &err) {
						expect(err.offending_feature)
							.to_be(phys::vclip::edge(0, 1)).orr()
							.to_be(phys::vclip::edge(1, 2)).orr()
							.to_be(phys::vclip::edge(2, 0));
					}
				});

				it("fails to validate a polyhedron with a missing face (an open polyhedron)", []() {
					// A pyramid, but the bottom face has been replaced with three non-convex faces
					phys::vclip::polyhedron p{
						.vertices = {
							phys::vclip::vertex(phys::vec3(1.0_r, 0.0_r, 0.0_r), 0),
							phys::vclip::vertex(phys::vec3(0.0_r, 0.0_r, 1.0_r), 1),
							phys::vclip::vertex(phys::vec3(0.0_r, 0.0_r, -1.0_r), 2),
							phys::vclip::vertex(phys::vec3(0.0_r, 1.0_r, 0.0_r), 3),
							phys::vclip::vertex(phys::vec3(0.0_r, 0.1_r, 0.0_r), 4)
						},
						.edges = {
							phys::vclip::edge(0, 1),
							phys::vclip::edge(1, 2),
							phys::vclip::edge(2, 0),
							phys::vclip::edge(0, 3),
							phys::vclip::edge(1, 3),
							phys::vclip::edge(2, 3),
							phys::vclip::edge(0, 4),
							phys::vclip::edge(1, 4),
							phys::vclip::edge(2, 4)
						},
						.faces = {
							phys::vclip::face({ 0, 3, 1 }),
							phys::vclip::face({ 1, 3, 2 }),
							phys::vclip::face({ 2, 3, 0 }),
							phys::vclip::face({ 0, 1, 4 }),
							phys::vclip::face({ 2, 4, 1 }),
							phys::vclip::face({ 0, 4, 2 })
						}
					};

					try {
						// TODO: Callable matcher with `to_throw`
						p.validate();
						fail("Expected polyhedron validation to fail");
					} catch (const phys::vclip::geometry_error &err) {
						expect(err.offending_feature)
							.to_be(phys::vclip::edge(0, 4)).orr()
							.to_be(phys::vclip::edge(1, 4)).orr()
							.to_be(phys::vclip::edge(2, 4));
					}
				});

				describe("for a box", []() {
					before_all([&]() {
						box_body_1.pos = phys::vec3(0.0_r);
						box_1.half_size = phys::vec3(1.0_r, 2.0_r, 3.0_r);
					});

					it("converts a box to a polyhedron with the correct number of features", [&]() {
						phys::vclip::polyhedron p = box_1.to_polyhedron();

						expect(p.vertices).to_have_size(8);
						expect(p.edges).to_have_size(12);
						expect(p.faces).to_have_size(6);
						expect(p.euler_characteristic()).to_be(2);

						p.validate();
					});

					it("gets edges adjacent to a vertex", [&]() {
						phys::vclip::polyhedron p = box_1.to_polyhedron();
						auto edges = p.vertices[0].edges(p);

						expect(edges).to_have_size(3).annd()
							.to_have_item(phys::vclip::edge(0, 1)).annd()
							.to_have_item(phys::vclip::edge(0, 2)).annd()
							.to_have_item(phys::vclip::edge(0, 4));

						edges = p.vertices[5].edges(p);

						expect(edges).to_have_size(3).annd()
							.to_have_item(phys::vclip::edge(1, 5)).annd()
							.to_have_item(phys::vclip::edge(5, 7)).annd()
							.to_have_item(phys::vclip::edge(4, 5));
					});

					it("gets vertices adjacent to an edge", [&]() {
						phys::vclip::polyhedron p = box_1.to_polyhedron();
						phys::vclip::edge e(3, 7);
						auto verts = e.vertices(p);

						expect(verts).to_have_size(2).annd()
							.to_have_item(phys::vclip::vertex(
								phys::vec3(1.0_r, -1.0_r, -1.0_r), 3
							)).annd()
							.to_have_item(phys::vclip::vertex(
								phys::vec3(-1.0_r), 7
							));
					});

					it("gets faces adjacent to an edge", [&]() {
						phys::vclip::polyhedron p = box_1.to_polyhedron();
						phys::vclip::edge e(3, 7);
						auto faces = e.faces(p);

						expect(faces).to_have_size(2).annd()
							.to_have_item(phys::vclip::face({
								7, 3, 2, 6
							})).annd()
							.to_have_item(phys::vclip::face({
								7, 5, 1, 3
							}));
					});

					it("gets vertices adjacent to a face", [&]() {
						phys::vclip::polyhedron p = box_1.to_polyhedron();
						phys::vclip::face f({ 0, 1, 5, 4 });

						auto verts = f.vertices(p);

						expect(verts).to_have_size(4).annd()
							.to_have_item(phys::vclip::vertex(phys::vec3(1.0_r), 0)).annd()
							.to_have_item(phys::vclip::vertex(phys::vec3(1.0_r, 1.0_r, -1.0_r), 1)).annd()
							.to_have_item(phys::vclip::vertex(phys::vec3(-1.0_r, 1.0_r, -1.0_r), 5)).annd()
							.to_have_item(phys::vclip::vertex(phys::vec3(-1.0_r, 1.0_r, 1.0_r), 4));
					});

					it("gets edges adjacent to a face", [&]() {
						phys::vclip::polyhedron p = box_1.to_polyhedron();
						phys::vclip::face f({ 0, 4, 6, 2 });

						expect(p.faces).to_have_item(f).annd()
							.naht().to_have_item(phys::vclip::face({ 0, 2, 6, 4 }));

						auto edges = f.edges();

						expect(edges).to_have_size(4).annd()
							.to_have_item(phys::vclip::edge(0, 4)).annd()
							.to_have_item(phys::vclip::edge(4, 6)).annd()
							.to_have_item(phys::vclip::edge(6, 2)).annd()
							.to_have_item(phys::vclip::edge(2, 0));
					});

					it("computes face normals", [&]() {
						phys::vclip::polyhedron p = box_1.to_polyhedron();

						expect(phys::vclip::face({ 0, 1, 5, 4 }).normal(p)).to_be(
							phys::vec3(0.0_r, 1.0_r, 0.0_r)
						);
						expect(phys::vclip::face({ 0, 4, 6, 2 }).normal(p)).to_be(
							phys::vec3(0.0_r, 0.0_r, 1.0_r)
						);
						expect(phys::vclip::face({ 0, 2, 3, 1 }).normal(p)).to_be(
							phys::vec3(1.0_r, 0.0_r, 0.0_r)
						);
						expect(phys::vclip::face({ 7, 3, 2, 6 }).normal(p)).to_be(
							phys::vec3(0.0_r, -1.0_r, 0.0_r)
						);
						expect(phys::vclip::face({ 1, 3, 7, 5 }).normal(p)).to_be(
							phys::vec3(0.0_r, 0.0_r, -1.0_r)
						);
						expect(phys::vclip::face({ 7, 6, 4, 5 }).normal(p)).to_be(
							phys::vec3(-1.0_r, 0.0_r, 0.0_r)
						);
					});

					it("computes V-E planes from vertex", [&]() {
						phys::vclip::polyhedron p = box_1.to_polyhedron();
						auto vplanes = p.vertices[4].ve_planes(p);

						expect(vplanes).to_have_size(3).annd()
							.to_have_item(phys::vclip::vplane(
								p.vertices[4], phys::vclip::edge(4, 0),
								phys::vec3(-1.0_r, 2.0_r, 3.0_r),
								phys::vec3(1.0_r, 0.0_r, 0.0_r)
							)).annd()
							.to_have_item(phys::vclip::vplane(
								p.vertices[4], phys::vclip::edge(4, 5),
								phys::vec3(-1.0_r, 2.0_r, 3.0_r),
								phys::vec3(0.0_r, 0.0_r, -1.0_r)
							)).annd()
							.to_have_item(phys::vclip::vplane(
								p.vertices[4], phys::vclip::edge(4, 6),
								phys::vec3(-1.0_r, 2.0_r, 3.0_r),
								phys::vec3(0.0_r, -1.0_r, 0.0_r)
							));
					});

					it("computes V-E planes from edge", [&]() {
						phys::vclip::polyhedron p = box_1.to_polyhedron();
						auto vplanes = phys::vclip::edge(1, 5).ve_planes(p);

						expect(vplanes).to_have_size(2).annd()
							.to_have_item(phys::vclip::vplane(
								p.vertices[1], phys::vclip::edge(1, 5),
								phys::vec3(1.0_r, 2.0_r, -3.0_r),
								phys::vec3(1.0_r, 0.0_r, 0.0_r)
							)).annd()
							.to_have_item(phys::vclip::vplane(
								p.vertices[5], phys::vclip::edge(1, 5),
								phys::vec3(-1.0_r, 2.0_r, -3.0_r),
								phys::vec3(-1.0_r, 0.0_r, 0.0_r)
							));
					});

					it("computes F-E planes from edge", [&]() {
						phys::vclip::polyhedron p = box_1.to_polyhedron();
						auto vplanes = phys::vclip::edge(1, 5).fe_planes(p);
						phys::vclip::face f1({ 0, 1, 5, 4 });
						phys::vclip::face f2({ 1, 3, 7, 5 });
						phys::vclip::edge e(1, 5);

						expect(vplanes).to_have_size(2)
							.annd()
								.to_have_item(phys::vclip::vplane(
									f1,
									e,
									phys::vec3(-1.0_r, 2.0_r, -3.0_r),
									phys::vec3(0.0_r, 0.0_r, 1.0_r)
								)).orr()
								.to_have_item(phys::vclip::vplane(
									f1,
									e,
									phys::vec3(1.0_r, 2.0_r, -3.0_r),
									phys::vec3(0.0_r, 0.0_r, 1.0_r)
								))
							.annd()
								.to_have_item(phys::vclip::vplane(
									f2,
									e,
									phys::vec3(-1.0_r, 2.0_r, -3.0_r),
									phys::vec3(0.0_r, -1.0_r, 0.0_r)
								)).orr()
								.to_have_item(phys::vclip::vplane(
									f2,
									e,
									phys::vec3(1.0_r, 2.0_r, -3.0_r),
									phys::vec3(0.0_r, -1.0_r, 0.0_r)
								));
					});

					it("computes F-E planes from face", [&]() {
						phys::vclip::polyhedron p = box_1.to_polyhedron();
						phys::vclip::face f({ 1, 5, 4, 0 });
						auto vplanes = f.fe_planes(p);

						expect(vplanes).to_have_size(4)
							.annd()
								.to_have_item(phys::vclip::vplane(
									f,
									phys::vclip::edge(1, 5),
									phys::vec3(1.0_r, 2.0_r, -3.0_r),
									phys::vec3(0.0_r, 0.0_r, -1.0_r)
								)).orr()
								.to_have_item(phys::vclip::vplane(
									f,
									phys::vclip::edge(1, 5),
									phys::vec3(-1.0_r, 2.0_r, -3.0_r),
									phys::vec3(0.0_r, 0.0_r, -1.0_r)
								))
							.annd()
								.to_have_item(phys::vclip::vplane(
									f,
									phys::vclip::edge(5, 4),
									phys::vec3(-1.0_r, 2.0_r, -3.0_r),
									phys::vec3(-1.0_r, 0.0_r, 0.0_r)
								)).orr()
								.to_have_item(phys::vclip::vplane(
									f,
									phys::vclip::edge(5, 4),
									phys::vec3(-1.0_r, 2.0_r, 3.0_r),
									phys::vec3(-1.0_r, 0.0_r, 0.0_r)
								))
							.annd()
								.to_have_item(phys::vclip::vplane(
									f,
									phys::vclip::edge(4, 0),
									phys::vec3(-1.0_r, 2.0_r, 3.0_r),
									phys::vec3(0.0_r, 0.0_r, 1.0_r)
								)).orr()
								.to_have_item(phys::vclip::vplane(
									f,
									phys::vclip::edge(4, 0),
									phys::vec3(1.0_r, 2.0_r, 3.0_r),
									phys::vec3(0.0_r, 0.0_r, 1.0_r)
								))
							.annd()
								.to_have_item(phys::vclip::vplane(
									f,
									phys::vclip::edge(0, 1),
									phys::vec3(1.0_r, 2.0_r, 3.0_r),
									phys::vec3(1.0_r, 0.0_r, 0.0_r)
								)).orr()
								.to_have_item(phys::vclip::vplane(
									f,
									phys::vclip::edge(0, 1),
									phys::vec3(1.0_r, 2.0_r, -3.0_r),
									phys::vec3(1.0_r, 0.0_r, 0.0_r)
								));
					});
				});

				describe("for two boxes", []() {
					describe("clip_edge", []() {
						describe("simple exclusion", []() {
							after_each([&]() {
								box_body_1 = {};
								box_body_2 = {};
								box_1.half_size = phys::vec3(1.0_r);
								box_2.half_size = phys::vec3(1.0_r);
							});

							it("selects the face when a clipped edge is simply excluded by an edge's F-E plane", [&]() {
								box_body_2.pos = phys::vec3(0.0_r, 2.0_r, 0.0_r);
								box_body_2.calculate_derived_data();
								phys::vclip::polyhedron p1 = box_1.to_polyhedron();
								phys::vclip::polyhedron p2 = box_2.to_polyhedron();
								phys::vclip::edge f(0, 1);
								phys::vclip::edge e(7, 6);
								std::vector<phys::vclip::vplane> vps = f.fe_planes(p1);

								phys::vclip::clip_result cr = phys::vclip::clip_edge(
									p2,
									e,
									f,
									vps
								);

								expect(cr).to_be(phys::vclip::clip_result(
									e,
									f,
									phys::vclip::face({ 0, 1, 5, 4 }),
									phys::vclip::face({ 0, 1, 5, 4 }),
									0.0_r,
									1.0_r,
									false
								));
							});

							it("selects the face when a clipped edge is simply excluded by an edge's F-E plane "
								"and the clipped edge is reversed", [&]() {
								box_body_2.pos = phys::vec3(0.0_r, 2.0_r, 0.0_r);
								box_body_2.calculate_derived_data();
								phys::vclip::polyhedron p1 = box_1.to_polyhedron();
								phys::vclip::polyhedron p2 = box_2.to_polyhedron();
								phys::vclip::edge f(0, 1);
								phys::vclip::edge e(6, 7);
								std::vector<phys::vclip::vplane> vps = f.fe_planes(p1);

								phys::vclip::clip_result cr = phys::vclip::clip_edge(
									p2,
									e,
									f,
									vps
								);

								expect(cr).to_be(phys::vclip::clip_result(
									e,
									f,
									phys::vclip::face({ 0, 1, 5, 4 }),
									phys::vclip::face({ 0, 1, 5, 4 }),
									0.0_r,
									1.0_r,
									false
								));
							});

							it("selects the edge when a clipped edge is excluded by a face's F-E plane", [&]() {
								box_body_2.pos = phys::vec3(0.5_r, 1.0_r, 2.5_r);
								box_body_2.calculate_derived_data();
								phys::vclip::polyhedron p1 = box_1.to_polyhedron();
								phys::vclip::polyhedron p2 = box_2.to_polyhedron();
								phys::vclip::face f({ 0, 1, 5, 4 });
								phys::vclip::edge e(7, 6);
								std::vector<phys::vclip::vplane> vps = f.fe_planes(p1);

								phys::vclip::clip_result cr = phys::vclip::clip_edge(
									p2,
									e,
									f,
									vps
								);

								expect(cr).to_be(phys::vclip::clip_result(
									e,
									f,
									phys::vclip::edge(0, 4),
									phys::vclip::edge(0, 4),
									0.0_r,
									1.0_r,
									false
								));
							});

							it("selects the edge when a clipped edge is excluded by a face's F-E plane "
								"and the clipped edge is reversed", [&]() {
								box_body_2.pos = phys::vec3(0.5_r, 1.0_r, 2.5_r);
								box_body_2.calculate_derived_data();
								phys::vclip::polyhedron p1 = box_1.to_polyhedron();
								phys::vclip::polyhedron p2 = box_2.to_polyhedron();
								phys::vclip::face f({ 0, 1, 5, 4 });
								phys::vclip::edge e(6, 7);
								std::vector<phys::vclip::vplane> vps = f.fe_planes(p1);

								phys::vclip::clip_result cr = phys::vclip::clip_edge(
									p2,
									e,
									f,
									vps
								);

								expect(cr).to_be(phys::vclip::clip_result(
									e,
									f,
									phys::vclip::edge(0, 4),
									phys::vclip::edge(0, 4),
									0.0_r,
									1.0_r,
									false
								));
							});

							it("selects the edge when a clipped edge is simply excluded by a vertex's V-E plane", [&]() {
								box_body_2.pos = phys::vec3(0.5_r, 2.0_r, 2.5_r);
								box_body_2.calculate_derived_data();
								phys::vclip::polyhedron p1 = box_1.to_polyhedron();
								phys::vclip::polyhedron p2 = box_2.to_polyhedron();
								phys::vclip::vertex f = p1.vertices[4];
								phys::vclip::edge e(7, 6);
								std::vector<phys::vclip::vplane> vps = f.ve_planes(p1);

								phys::vclip::clip_result cr = phys::vclip::clip_edge(
									p2,
									e,
									f,
									vps
								);

								expect(cr).to_be(phys::vclip::clip_result(
									e,
									f,
									phys::vclip::edge(0, 4),
									phys::vclip::edge(0, 4),
									0.0_r,
									1.0_r,
									false
								));
							});

							it("selects the edge when a clipped edge is simply excluded by a vertex's V-E plane "
								"and the clipped edge is reversed", [&]() {
								box_body_2.pos = phys::vec3(0.5_r, 2.0_r, 2.5_r);
								box_body_2.calculate_derived_data();
								phys::vclip::polyhedron p1 = box_1.to_polyhedron();
								phys::vclip::polyhedron p2 = box_2.to_polyhedron();
								phys::vclip::vertex f = p1.vertices[4];
								phys::vclip::edge e(6, 7);
								std::vector<phys::vclip::vplane> vps = f.ve_planes(p1);

								phys::vclip::clip_result cr = phys::vclip::clip_edge(
									p2,
									e,
									f,
									vps
								);

								expect(cr).to_be(phys::vclip::clip_result(
									e,
									f,
									phys::vclip::edge(0, 4),
									phys::vclip::edge(0, 4),
									0.0_r,
									1.0_r,
									false
								));
							});

							it("selects the vertex when a clipped edge is simply excluded by an edge's V-E plane", [&]() {
								box_body_2.pos = phys::vec3(0.5_r, 2.0_r, 2.5_r);
								box_body_2.calculate_derived_data();
								phys::vclip::polyhedron p1 = box_1.to_polyhedron();
								phys::vclip::polyhedron p2 = box_2.to_polyhedron();
								phys::vclip::edge f(0, 1);
								phys::vclip::edge e(7, 6);
								std::vector<phys::vclip::vplane> vps = f.ve_planes(p1);

								phys::vclip::clip_result cr = phys::vclip::clip_edge(
									p2,
									e,
									f,
									vps
								);

								expect(cr).to_be(phys::vclip::clip_result(
									e,
									f,
									p1.vertices[0],
									p1.vertices[0],
									0.0_r,
									1.0_r,
									false
								));
							});

							it("selects the vertex when a clipped edge is simply excluded by an edge's V-E plane "
								"and the clipped edge is reversed", [&]() {
								box_body_2.pos = phys::vec3(0.5_r, 2.0_r, 2.5_r);
								box_body_2.calculate_derived_data();
								phys::vclip::polyhedron p1 = box_1.to_polyhedron();
								phys::vclip::polyhedron p2 = box_2.to_polyhedron();
								phys::vclip::edge f(0, 1);
								phys::vclip::edge e(6, 7);
								std::vector<phys::vclip::vplane> vps = f.ve_planes(p1);

								phys::vclip::clip_result cr = phys::vclip::clip_edge(
									p2,
									e,
									f,
									vps
								);

								expect(cr).to_be(phys::vclip::clip_result(
									e,
									f,
									p1.vertices[0],
									p1.vertices[0],
									0.0_r,
									1.0_r,
									false
								));
							});
						});

						describe("compound exclusion", []() {
							after_each([&]() {
								box_body_1 = {};
								box_body_2 = {};
								box_1.half_size = phys::vec3(1.0_r);
								box_2.half_size = phys::vec3(1.0_r);
							});

							it("selects faces when a clipped edge is excluded by an edge's F-E planes", [&]() {
								box_body_2.rot = make_rot((phys::real)M_PI / 4.0_r, phys::vec3(1.0_r, 0.0_r, 0.0_r));
								box_body_2.calculate_derived_data();
								phys::vclip::polyhedron p1 = box_1.to_polyhedron();
								phys::vclip::polyhedron p2 = box_2.to_polyhedron();
								phys::vclip::edge f(1, 5);
								phys::vclip::edge e(5, 7);
								std::vector<phys::vclip::vplane> vps = f.fe_planes(p1);

								phys::vclip::clip_result cr = phys::vclip::clip_edge(
									p2,
									e,
									f,
									vps
								);

								expect(cr).to_be(phys::vclip::clip_result(
									e,
									f,
									phys::vclip::face({ 0, 1, 5, 4 }),
									phys::vclip::face({ 1, 3, 7, 5 }),
									1.0_r / std::sqrt(2.0_r),
									1.0_r - 1.0_r / std::sqrt(2.0_r),
									false
								));
							});

							it("selects faces when a clipped edge is excluded by an edge's F-E planes "
								"and the clipped edge is reversed", [&]() {
								box_body_2.rot = make_rot((phys::real)M_PI / 4.0_r, phys::vec3(1.0_r, 0.0_r, 0.0_r));
								box_body_2.calculate_derived_data();
								phys::vclip::polyhedron p1 = box_1.to_polyhedron();
								phys::vclip::polyhedron p2 = box_2.to_polyhedron();
								phys::vclip::edge f(1, 5);
								phys::vclip::edge e(7, 5);
								std::vector<phys::vclip::vplane> vps = f.fe_planes(p1);

								phys::vclip::clip_result cr = phys::vclip::clip_edge(
									p2,
									e,
									f,
									vps
								);

								expect(cr).to_be(phys::vclip::clip_result(
									e,
									f,
									phys::vclip::face({ 1, 3, 7, 5 }),
									phys::vclip::face({ 0, 1, 5, 4 }),
									1.0_r / std::sqrt(2.0_r),
									1.0_r - 1.0_r / std::sqrt(2.0_r),
									false
								));
							});

							it("selects edges when a clipped edge is excluded by a face's F-E planes", [&]() {
								box_body_2.rot = make_rot((phys::real)M_PI / 4.0_r, phys::vec3(0.0_r, 1.0_r, 0.0_r));
								box_body_2.pos = phys::vec3(-0.5_r, 0.0_r, -0.5_r);
								box_body_2.calculate_derived_data();
								phys::vclip::polyhedron p1 = box_1.to_polyhedron();
								phys::vclip::polyhedron p2 = box_2.to_polyhedron();
								phys::vclip::face f({ 0, 1, 5, 4 });
								phys::vclip::edge e(1, 5);
								std::vector<phys::vclip::vplane> vps = f.fe_planes(p1);

								phys::vclip::clip_result cr = phys::vclip::clip_edge(
									p2,
									e,
									f,
									vps
								);

								expect(cr).to_be(phys::vclip::clip_result(
									e,
									f,
									phys::vclip::edge(1, 5),
									phys::vclip::edge(5, 4),
									1.0_r - std::sqrt(2.0_r) / 4.0_r,
									std::sqrt(2.0_r) / 4.0_r,
									false
								));
							});

							it("selects edges when a clipped edge is excluded by a face's F-E planes "
								"and the clipped edge is reversed", [&]() {
								box_body_2.rot = make_rot((phys::real)M_PI / 4.0_r, phys::vec3(0.0_r, 1.0_r, 0.0_r));
								box_body_2.pos = phys::vec3(-0.5_r, 0.0_r, -0.5_r);
								box_body_2.calculate_derived_data();
								phys::vclip::polyhedron p1 = box_1.to_polyhedron();
								phys::vclip::polyhedron p2 = box_2.to_polyhedron();
								phys::vclip::face f({ 0, 1, 5, 4 });
								phys::vclip::edge e(5, 1);
								std::vector<phys::vclip::vplane> vps = f.fe_planes(p1);

								phys::vclip::clip_result cr = phys::vclip::clip_edge(
									p2,
									e,
									f,
									vps
								);

								expect(cr).to_be(phys::vclip::clip_result(
									e,
									f,
									phys::vclip::edge(5, 4),
									phys::vclip::edge(1, 5),
									1.0_r - std::sqrt(2.0_r) / 4.0_r,
									std::sqrt(2.0_r) / 4.0_r,
									false
								));
							});

							it("selects edges when a clipped edge is excluded by a vertex's V-E planes", [&]() {
								box_body_2.rot = make_rot((phys::real)M_PI / 4.0_r, phys::vec3(0.0_r, 1.0_r, 0.0_r));
								box_body_2.pos = phys::vec3(1.5_r, 1.0_r, 1.5_r);
								box_body_2.calculate_derived_data();
								phys::vclip::polyhedron p1 = box_1.to_polyhedron();
								phys::vclip::polyhedron p2 = box_2.to_polyhedron();
								phys::vclip::vertex f = p1.vertices[0];
								phys::vclip::edge e(1, 5);
								std::vector<phys::vclip::vplane> vps = f.ve_planes(p1);

								phys::vclip::clip_result cr = phys::vclip::clip_edge(
									p2,
									e,
									f,
									vps
								);

								expect(cr).to_be(phys::vclip::clip_result(
									e,
									f,
									phys::vclip::edge(0, 1),
									phys::vclip::edge(4, 0),
									1.0_r - std::sqrt(2.0_r) / 4.0_r,
									std::sqrt(2.0_r) / 4.0_r,
									false
								));
							});

							it("selects edges when a clipped edge is excluded by a vertex's V-E planes "
								"and the clipped edge is reversed", [&]() {
								box_body_2.rot = make_rot((phys::real)M_PI / 4.0_r, phys::vec3(0.0_r, 1.0_r, 0.0_r));
								box_body_2.pos = phys::vec3(1.5_r, 1.0_r, 1.5_r);
								box_body_2.calculate_derived_data();
								phys::vclip::polyhedron p1 = box_1.to_polyhedron();
								phys::vclip::polyhedron p2 = box_2.to_polyhedron();
								phys::vclip::vertex f = p1.vertices[0];
								phys::vclip::edge e(5, 1);
								std::vector<phys::vclip::vplane> vps = f.ve_planes(p1);

								phys::vclip::clip_result cr = phys::vclip::clip_edge(
									p2,
									e,
									f,
									vps
								);

								expect(cr).to_be(phys::vclip::clip_result(
									e,
									f,
									phys::vclip::edge(4, 0),
									phys::vclip::edge(0, 1),
									1.0_r - std::sqrt(2.0_r) / 4.0_r,
									std::sqrt(2.0_r) / 4.0_r,
									false
								));
							});
						});

						describe("clipping on one side", []() {
							after_each([&]() {
								box_body_1 = {};
								box_body_2 = {};
								box_1.half_size = phys::vec3(1.0_r);
								box_2.half_size = phys::vec3(1.0_r);
							});

							it("clips an edge to the nearest F-E plane for an edge and selects the face", [&]() {
								box_body_2.pos = phys::vec3(0.5_r, 2.5_r, 0.5_r);
								box_body_2.calculate_derived_data();
								phys::vclip::polyhedron p1 = box_1.to_polyhedron();
								phys::vclip::polyhedron p2 = box_2.to_polyhedron();
								phys::vclip::edge e(7, 6);
								phys::vclip::edge f(0, 4);
								std::vector<phys::vclip::vplane> vps = f.fe_planes(p1);

								phys::vclip::clip_result cr = phys::vclip::clip_edge(
									p2,
									e,
									f,
									vps
								);

								expect(cr).to_be(phys::vclip::clip_result(
									e,
									f,
									phys::vclip::face({ 0, 1, 5, 4 }),
									{},
									0.75_r,
									1.0_r,
									true
								));
							});

							it("clips an edge to the nearest F-E plane for an edge "
								"and selects the face when the edge is reversed", [&]() {
								box_body_2.pos = phys::vec3(0.5_r, 2.5_r, 0.5_r);
								box_body_2.calculate_derived_data();
								phys::vclip::polyhedron p1 = box_1.to_polyhedron();
								phys::vclip::polyhedron p2 = box_2.to_polyhedron();
								phys::vclip::edge e(6, 7);
								phys::vclip::edge f(0, 4);
								std::vector<phys::vclip::vplane> vps = f.fe_planes(p1);

								phys::vclip::clip_result cr = phys::vclip::clip_edge(
									p2,
									e,
									f,
									vps
								);

								expect(cr).to_be(phys::vclip::clip_result(
									e,
									f,
									{},
									phys::vclip::face({ 0, 1, 5, 4 }),
									0.0_r,
									0.25_r,
									true
								));
							});

							it("clips an edge to the nearest F-E plane for a face and selects the edge", [&]() {
								box_body_2.pos = phys::vec3(0.0_r, 2.5_r, 0.5_r);
								box_body_2.calculate_derived_data();
								phys::vclip::polyhedron p1 = box_1.to_polyhedron();
								phys::vclip::polyhedron p2 = box_2.to_polyhedron();
								phys::vclip::edge e(7, 6);
								phys::vclip::face f({ 0, 1, 5, 4 });
								std::vector<phys::vclip::vplane> vps = f.fe_planes(p1);

								phys::vclip::clip_result cr = phys::vclip::clip_edge(
									p2,
									e,
									f,
									vps
								);

								expect(cr).to_be(phys::vclip::clip_result(
									e,
									f,
									{},
									phys::vclip::edge(0, 4),
									0.0_r,
									0.75_r,
									true
								));
							});

							it("clips an edge to the nearest F-E plane for a face and selects the edge "
								"when the clipped edge is reversed", [&]() {
								box_body_2.pos = phys::vec3(0.0_r, 2.5_r, 0.5_r);
								box_body_2.calculate_derived_data();
								phys::vclip::polyhedron p1 = box_1.to_polyhedron();
								phys::vclip::polyhedron p2 = box_2.to_polyhedron();
								phys::vclip::edge e(6, 7);
								phys::vclip::face f({ 0, 1, 5, 4 });
								std::vector<phys::vclip::vplane> vps = f.fe_planes(p1);

								phys::vclip::clip_result cr = phys::vclip::clip_edge(
									p2,
									e,
									f,
									vps
								);

								expect(cr).to_be(phys::vclip::clip_result(
									e,
									f,
									phys::vclip::edge(0, 4),
									{},
									0.25_r,
									1.0_r,
									true
								));
							});

							it("clips an edge to the nearest V-E plane for a vertex and selects the edge", [&]() {
								box_body_2.pos = phys::vec3(2.5_r, 2.5_r, 0.5_r);
								box_body_2.calculate_derived_data();
								phys::vclip::polyhedron p1 = box_1.to_polyhedron();
								phys::vclip::polyhedron p2 = box_2.to_polyhedron();
								phys::vclip::vertex f = p1.vertices[0];
								phys::vclip::edge e(7, 6);
								std::vector<phys::vclip::vplane> vps = f.ve_planes(p1);

								phys::vclip::clip_result cr = phys::vclip::clip_edge(
									p2,
									e,
									f,
									vps
								);

								expect(cr).to_be(phys::vclip::clip_result(
									e,
									f,
									phys::vclip::edge(0, 1),
									{},
									0.75_r,
									1.0_r,
									true
								));
							});

							it("clips an edge to the nearest V-E plane for a vertex and selects the edge "
								"when the clipped edge is reversed",[&]() {

								box_body_2.pos = phys::vec3(2.5_r, 2.5_r, 0.5_r);
								box_body_2.calculate_derived_data();
								phys::vclip::polyhedron p1 = box_1.to_polyhedron();
								phys::vclip::polyhedron p2 = box_2.to_polyhedron();
								phys::vclip::vertex f = p1.vertices[0];
								phys::vclip::edge e(6, 7);
								std::vector<phys::vclip::vplane> vps = f.ve_planes(p1);

								phys::vclip::clip_result cr = phys::vclip::clip_edge(
									p2,
									e,
									f,
									vps
								);

								expect(cr).to_be(phys::vclip::clip_result(
									e,
									f,
									{},
									phys::vclip::edge(0, 1),
									0.0_r,
									0.25_r,
									true
								));
							});

							it("clips an edge to the nearest V-E plane for an edge and selects the vertex", [&]() {
								box_body_2.pos = phys::vec3(2.5_r, 2.5_r, 0.5_r);
								box_body_2.calculate_derived_data();
								phys::vclip::polyhedron p1 = box_1.to_polyhedron();
								phys::vclip::polyhedron p2 = box_2.to_polyhedron();
								phys::vclip::edge f(1, 0);
								phys::vclip::edge e(7, 6);
								std::vector<phys::vclip::vplane> vps = f.ve_planes(p1);

								phys::vclip::clip_result cr = phys::vclip::clip_edge(
									p2,
									e,
									f,
									vps
								);

								expect(cr).to_be(phys::vclip::clip_result(
									e,
									f,
									{},
									p1.vertices[0],
									0.0_r,
									0.75_r,
									true
								));
							});

							it("clips an edge to the nearest V-E plane for an edge and selects the vertex "
								"when the clipped edge is reversed", [&]() {
								box_body_2.pos = phys::vec3(2.5_r, 2.5_r, 0.5_r);
								box_body_2.calculate_derived_data();
								phys::vclip::polyhedron p1 = box_1.to_polyhedron();
								phys::vclip::polyhedron p2 = box_2.to_polyhedron();
								phys::vclip::edge f(1, 0);
								phys::vclip::edge e(6, 7);
								std::vector<phys::vclip::vplane> vps = f.ve_planes(p1);

								phys::vclip::clip_result cr = phys::vclip::clip_edge(
									p2,
									e,
									f,
									vps
								);

								expect(cr).to_be(phys::vclip::clip_result(
									e,
									f,
									p1.vertices[0],
									{},
									0.25_r,
									1.0_r,
									true
								));
							});
						});

						describe("clipping on both sides", []() {
							after_each([&]() {
								box_body_1 = {};
								box_body_2 = {};
								box_1.half_size = phys::vec3(1.0_r);
								box_2.half_size = phys::vec3(1.0_r);
							});

							it("clips an edge to another edge's F-E planes and selects faces", [&]() {
								box_body_2.rot = make_rot((phys::real)M_PI / 4.0_r, phys::vec3(1.0_r, 0.0_r, 0.0_r));
								box_body_2.pos = phys::vec3(0.5_r, 0.5_r, -0.5_r);
								box_body_2.calculate_derived_data();
								phys::vclip::polyhedron p1 = box_1.to_polyhedron();
								phys::vclip::polyhedron p2 = box_2.to_polyhedron();
								phys::vclip::edge f(5, 1);
								phys::vclip::edge e(5, 7);
								std::vector<phys::vclip::vplane> vps = f.fe_planes(p1);

								phys::vclip::clip_result cr = phys::vclip::clip_edge(
									p2,
									e,
									f,
									vps
								);

								expect(cr).to_be(phys::vclip::clip_result(
									e,
									f,
									phys::vclip::face({ 0, 1, 5, 4 }),
									phys::vclip::face({ 1, 3, 7, 5 }),
									std::sqrt(2.0_r) / 4.0_r,
									1.0_r - std::sqrt(2.0_r) / 4.0_r,
									true
								));
							});

							it("clips an edge to another edge's F-E planes and selects faces "
								"when the clipped edge is reversed", [&]() {
								box_body_2.rot = make_rot((phys::real)M_PI / 4.0_r, phys::vec3(1.0_r, 0.0_r, 0.0_r));
								box_body_2.pos = phys::vec3(0.5_r, 0.5_r, -0.5_r);
								box_body_2.calculate_derived_data();
								phys::vclip::polyhedron p1 = box_1.to_polyhedron();
								phys::vclip::polyhedron p2 = box_2.to_polyhedron();
								phys::vclip::edge f(5, 1);
								phys::vclip::edge e(7, 5);
								std::vector<phys::vclip::vplane> vps = f.fe_planes(p1);

								phys::vclip::clip_result cr = phys::vclip::clip_edge(
									p2,
									e,
									f,
									vps
								);

								expect(cr).to_be(phys::vclip::clip_result(
									e,
									f,
									phys::vclip::face({ 1, 3, 7, 5 }),
									phys::vclip::face({ 0, 1, 5, 4 }),
									std::sqrt(2.0_r) / 4.0_r,
									1.0_r - std::sqrt(2.0_r) / 4.0_r,
									true
								));
							});

							it("clips an edge to a face's F-E planes and selects faces", [&]() {
								box_body_2.rot = make_rot((phys::real)M_PI / 4.0_r, phys::vec3(1.0_r, 0.0_r, 0.0_r));
								box_body_2.calculate_derived_data();
								phys::vclip::polyhedron p1 = box_1.to_polyhedron();
								phys::vclip::polyhedron p2 = box_2.to_polyhedron();
								phys::vclip::face f({ 4, 6, 7, 5 });
								phys::vclip::edge e(5, 4);
								std::vector<phys::vclip::vplane> vps = f.fe_planes(p1);

								phys::vclip::clip_result cr = phys::vclip::clip_edge(
									p2,
									e,
									f,
									vps
								);

								expect(cr).to_be(phys::vclip::clip_result(
									e,
									f,
									phys::vclip::edge(5, 4),
									phys::vclip::edge(4, 6),
									1.0_r - 1.0_r / std::sqrt(2.0_r),
									1.0_r / std::sqrt(2.0_r),
									true
								));
							});

							it("clips an edge to a face's F-E planes and selects faces "
								"when the clipped edge is reversed", [&]() {
								box_body_2.rot = make_rot((phys::real)M_PI / 4.0_r, phys::vec3(1.0_r, 0.0_r, 0.0_r));
								box_body_2.calculate_derived_data();
								phys::vclip::polyhedron p1 = box_1.to_polyhedron();
								phys::vclip::polyhedron p2 = box_2.to_polyhedron();
								phys::vclip::face f({ 4, 6, 7, 5 });
								phys::vclip::edge e(4, 5);
								std::vector<phys::vclip::vplane> vps = f.fe_planes(p1);

								phys::vclip::clip_result cr = phys::vclip::clip_edge(
									p2,
									e,
									f,
									vps
								);

								expect(cr).to_be(phys::vclip::clip_result(
									e,
									f,
									phys::vclip::edge(4, 6),
									phys::vclip::edge(5, 4),
									1.0_r - 1.0_r / std::sqrt(2.0_r),
									1.0_r / std::sqrt(2.0_r),
									true
								));
							});

							it("clips an edge to a vertex's V-E planes and selects the edges", [&]() {
								box_body_2.rot = make_rot((phys::real)M_PI / 4.0_r, phys::vec3(1.0_r, 0.0_r, 0.0_r));
								box_body_2.pos = phys::vec3(1.5_r, 0.5_r, -0.5_r);
								box_body_2.calculate_derived_data();
								phys::vclip::polyhedron p1 = box_1.to_polyhedron();
								phys::vclip::polyhedron p2 = box_2.to_polyhedron();
								phys::vclip::vertex f = p1.vertices[1];
								phys::vclip::edge e(1, 3);
								std::vector<phys::vclip::vplane> vps = f.ve_planes(p1);

								phys::vclip::clip_result cr = phys::vclip::clip_edge(
									p2,
									e,
									f,
									vps
								);

								expect(cr).to_be(phys::vclip::clip_result(
									e,
									f,
									phys::vclip::edge(0, 1),
									phys::vclip::edge(1, 3),
									std::sqrt(2.0_r) / 4.0_r,
									1.0_r - std::sqrt(2.0_r) / 4.0_r,
									true
								));
							});

							it("clips an edge to a vertex's V-E planes and selects the edges "
								"when the clipped edge is reversed", [&]() {
								box_body_2.rot = make_rot((phys::real)M_PI / 4.0_r, phys::vec3(1.0_r, 0.0_r, 0.0_r));
								box_body_2.pos = phys::vec3(1.5_r, 0.5_r, -0.5_r);
								box_body_2.calculate_derived_data();
								phys::vclip::polyhedron p1 = box_1.to_polyhedron();
								phys::vclip::polyhedron p2 = box_2.to_polyhedron();
								phys::vclip::vertex f = p1.vertices[1];
								phys::vclip::edge e(3, 1);
								std::vector<phys::vclip::vplane> vps = f.ve_planes(p1);

								phys::vclip::clip_result cr = phys::vclip::clip_edge(
									p2,
									e,
									f,
									vps
								);

								expect(cr).to_be(phys::vclip::clip_result(
									e,
									f,
									phys::vclip::edge(1, 3),
									phys::vclip::edge(0, 1),
									std::sqrt(2.0_r) / 4.0_r,
									1.0_r - std::sqrt(2.0_r) / 4.0_r,
									true
								));
							});

							it("clips an edge to an edge's V-E planes and selects the vertices", [&]() {
								box_body_2.pos = phys::vec3(3.0_r, 3.0_r, 0.0_r);
								box_body_2.calculate_derived_data();
								box_2.half_size = phys::vec3(2.0_r);
								phys::vclip::polyhedron p1 = box_1.to_polyhedron();
								phys::vclip::polyhedron p2 = box_2.to_polyhedron();
								phys::vclip::edge f(0, 1);
								phys::vclip::edge e(6, 7);
								std::vector<phys::vclip::vplane> vps = f.ve_planes(p1);

								phys::vclip::clip_result cr = phys::vclip::clip_edge(
									p2,
									e,
									f,
									vps
								);

								expect(cr).to_be(phys::vclip::clip_result(
									e,
									f,
									p1.vertices[0],
									p1.vertices[1],
									0.25_r,
									0.75_r,
									true
								));
							});

							it("clips an edge to an edge's V-E planes and selects the vertices "
								"when the clipped edge is reversed", [&]() {
								box_body_2.pos = phys::vec3(3.0_r, 3.0_r, 0.0_r);
								box_body_2.calculate_derived_data();
								box_2.half_size = phys::vec3(2.0_r);
								phys::vclip::polyhedron p1 = box_1.to_polyhedron();
								phys::vclip::polyhedron p2 = box_2.to_polyhedron();
								phys::vclip::edge f(0, 1);
								phys::vclip::edge e(7, 6);
								std::vector<phys::vclip::vplane> vps = f.ve_planes(p1);

								phys::vclip::clip_result cr = phys::vclip::clip_edge(
									p2,
									e,
									f,
									vps
								);

								expect(cr).to_be(phys::vclip::clip_result(
									e,
									f,
									p1.vertices[1],
									p1.vertices[0],
									0.25_r,
									0.75_r,
									true
								));
							});
						});
					});
				});
			});

			describe("state handlers", []() {
				describe("V-V state handler", []() {
					after_each([&]() {
						box_body_1 = {};
						box_body_2 = {};
						box_1.half_size = phys::vec3(1.0_r);
						box_2.half_size = phys::vec3(1.0_r);
					});

					it("terminates when the vertices are the closest points between two boxes and are in each others' Voronoi regions", [&]() {
						box_body_2.pos = phys::vec3(2.5_r);
						box_body_2.calculate_derived_data();
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();

						phys::vclip::algorithm_state state =
							phys::vclip::vv_state(
								p1,
								p2,
								p1.vertices[0],
								p2.vertices[7]
							);

						expect(state.step).to_be(phys::vclip::algorithm_step::Done);
						expect(state.f1).to_be(p1.vertices[0]);
						expect(state.f2).to_be(p2.vertices[7]);
					});

					it("updates to the nearest edge on the first polyhedron", [&]() {
						box_body_2.pos = phys::vec3(1.0_r, 2.5_r, 2.5_r);
						box_body_2.calculate_derived_data();
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();

						phys::vclip::algorithm_state state =
							phys::vclip::vv_state(
								p1,
								p2,
								p1.vertices[0],
								p2.vertices[7]
							);

						expect(state.step).to_be(phys::vclip::algorithm_step::Continue);
						expect(state.f1).to_be(phys::vclip::edge(0, 4));
						expect(state.f2).to_be(p2.vertices[7]);
					});

					it("updates to the nearest edge on the second polyhedron", [&]() {
						box_body_2.pos = phys::vec3(1.0_r, 2.5_r, 2.5_r);
						box_body_2.calculate_derived_data();
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();

						phys::vclip::algorithm_state state =
							phys::vclip::vv_state(
								p1,
								p2,
								p1.vertices[0],
								p2.vertices[3]
							);

						expect(state.step).to_be(phys::vclip::algorithm_step::Continue);
						expect(state.f1).to_be(p1.vertices[0]);
						expect(state.f2).to_be(phys::vclip::edge(3, 7));
					});

					it("updates to the nearest edge when the polyhedra are penetrating", [&]() {
						box_body_2.pos = phys::vec3(0.5_r, 0.8_r, 0.8_r);
						box_body_2.calculate_derived_data();
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();

						phys::vclip::algorithm_state state =
							phys::vclip::vv_state(
								p1,
								p2,
								p1.vertices[0],
								p2.vertices[3]
							);

						expect(state.step).to_be(phys::vclip::algorithm_step::Continue);
						// TODO: More assertions here
					});
				});

				describe("V-E state handler", []() {
					after_each([&]() {
						box_body_1 = {};
						box_body_2 = {};
						box_1.half_size = phys::vec3(1.0_r);
						box_2.half_size = phys::vec3(1.0_r);
					});

					it("selects the closest vertex for a vertex outside of the edge's V-E planes", [&]() {
						box_body_2.pos = phys::vec3(2.5_r);
						box_body_2.calculate_derived_data();
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::edge e(4, 0);
						phys::vclip::vertex v = p2.vertices[7];

						phys::vclip::algorithm_state state = phys::vclip::ve_state(
							p2,
							p1,
							v,
							e
						);

						expect(state).to_be(phys::vclip::algorithm_state{
							.f1 = v,
							.f2 = p1.vertices[0],
							.step = phys::vclip::algorithm_step::Continue,
							.penetration = 0.0_r
						});
					});

					it("selects the closest vertex for a vertex outside of the edge's V-E planes "
						"when the edge is reversed", [&]() {
						box_body_2.pos = phys::vec3(2.5_r);
						box_body_2.calculate_derived_data();
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::edge e(0, 4);
						phys::vclip::vertex v = p2.vertices[7];

						phys::vclip::algorithm_state state = phys::vclip::ve_state(
							p2,
							p1,
							v,
							e
						);

						expect(state).to_be(phys::vclip::algorithm_state{
							.f1 = v,
							.f2 = p1.vertices[0],
							.step = phys::vclip::algorithm_step::Continue,
							.penetration = 0.0_r
						});
					});

					it("selects the closest vertex for a vertex outside of the edge's V-E planes (2)", [&]() {
						box_body_2.pos = phys::vec3(-1.0_r, 2.5_r, 2.5_r);
						box_body_2.calculate_derived_data();
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::edge e(4, 0);
						phys::vclip::vertex v = p2.vertices[7];

						phys::vclip::algorithm_state state = phys::vclip::ve_state(
							p2,
							p1,
							v,
							e
						);

						expect(state).to_be(phys::vclip::algorithm_state{
							.f1 = v,
							.f2 = p1.vertices[4],
							.step = phys::vclip::algorithm_step::Continue,
							.penetration = 0.0_r
						});
					});

					it("selects the closest vertex for a vertex outside of the edge's V-E planes (2) "
						"when the edge is reversed", [&]() {
						box_body_2.pos = phys::vec3(-1.0_r, 2.5_r, 2.5_r);
						box_body_2.calculate_derived_data();
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::edge e(0, 4);
						phys::vclip::vertex v = p2.vertices[7];

						phys::vclip::algorithm_state state = phys::vclip::ve_state(
							p2,
							p1,
							v,
							e
						);

						expect(state).to_be(phys::vclip::algorithm_state{
							.f1 = v,
							.f2 = p1.vertices[4],
							.step = phys::vclip::algorithm_step::Continue,
							.penetration = 0.0_r
						});
					});

					it("selects the closest face for a vertex outside of the edge's F-E planes", [&]() {
						box_body_2.pos = phys::vec3(1.0_r, 2.5_r, 1.0_r);
						box_body_2.calculate_derived_data();
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::edge e(0, 4);
						phys::vclip::vertex v = p2.vertices[7];

						phys::vclip::algorithm_state state = phys::vclip::ve_state(
							p2,
							p1,
							v,
							e
						);

						expect(state).to_be(phys::vclip::algorithm_state{
							.f1 = v,
							.f2 = phys::vclip::face({ 0, 1, 5, 4 }),
							.step = phys::vclip::algorithm_step::Continue,
							.penetration = 0.0_r
						});
					});

					it("selects the closest face for a vertex outside of the edge's F-E planes "
						"when the edge is reversed", [&]() {
						box_body_2.pos = phys::vec3(1.0_r, 2.5_r, 1.0_r);
						box_body_2.calculate_derived_data();
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::edge e(4, 0);
						phys::vclip::vertex v = p2.vertices[7];

						phys::vclip::algorithm_state state = phys::vclip::ve_state(
							p2,
							p1,
							v,
							e
						);

						expect(state).to_be(phys::vclip::algorithm_state{
							.f1 = v,
							.f2 = phys::vclip::face({ 0, 1, 5, 4 }),
							.step = phys::vclip::algorithm_step::Continue,
							.penetration = 0.0_r
						});
					});

					it("selects the closest edge when the vertex is inside the edge's Voronoi region", [&]() {
						box_body_2.pos = phys::vec3(1.0_r);
						box_body_2.rot = make_rot((phys::real)M_PI / 16.0_r, phys::vec3(0.0_r, 1.0_r, 0.0_r));
						box_body_2.calculate_derived_data();
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::edge e(4, 0);
						phys::vclip::vertex v = p2.vertices[4];

						phys::vclip::algorithm_state state = phys::vclip::ve_state(
							p2,
							p1,
							v,
							e
						);

						expect(state).to_be(phys::vclip::algorithm_state{
							.f1 = phys::vclip::edge(5, 4),
							.f2 = e,
							.step = phys::vclip::algorithm_step::Continue,
							.penetration = 0.0_r
						});
					});

					it("selects the closest edge when the vertex is inside the edge's Voronoi region "
						"when the edge is reversed", [&]() {
						box_body_2.pos = phys::vec3(1.0_r);
						box_body_2.rot = make_rot((phys::real)M_PI / 16.0_r, phys::vec3(0.0_r, 1.0_r, 0.0_r));
						box_body_2.calculate_derived_data();
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::edge e(0, 4);
						phys::vclip::vertex v = p2.vertices[4];

						phys::vclip::algorithm_state state = phys::vclip::ve_state(
							p2,
							p1,
							v,
							e
						);

						expect(state).to_be(phys::vclip::algorithm_state{
							.f1 = phys::vclip::edge(5, 4),
							.f2 = e,
							.step = phys::vclip::algorithm_step::Continue,
							.penetration = 0.0_r
						});
					});

					// The algorithm might be expected to terminate here,
					// but it does not because the edge is clipped on both
					// sides by the vertex's Voronoi region. The algorithm doesn't
					// yet know that the vertex's neighbors are futher from the edge
					// than the vertex.
					it("selects the closest edge when the edge is inside the vertex's Voronoi region "
						"and is clipped on both sides", [&]() {
						box_body_2.pos = phys::vec3(0.0_r, 2.5_r, 2.5_r);
						box_body_2.rot = make_rot((phys::real)M_PI / 4.0_r, phys::vec3(0.0_r, 1.0_r, 0.0_r));
						box_body_2.calculate_derived_data();
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::edge e(0, 4);
						phys::vclip::vertex v = p2.vertices[3];

						phys::vclip::algorithm_state state = phys::vclip::ve_state(
							p2,
							p1,
							v,
							e
						);

						expect(state)
							.to_be(phys::vclip::algorithm_state{
								.f1 = phys::vclip::edge(2, 3),
								.f2 = e,
								.step = phys::vclip::algorithm_step::Continue,
								.penetration = 0.0_r
							}).orr().to_be(phys::vclip::algorithm_state{
								.f1 = phys::vclip::edge(3, 7),
								.f2 = e,
								.step = phys::vclip::algorithm_step::Continue,
								.penetration = 0.0_r
							});
					});

					it("terminates when the edge is inside the vertex's Voronoi region "
						"and is not clipped", [&]() {
						box_body_2.pos = phys::vec3(0.0_r, 10.0_r, 10.0_r);
						box_body_2.rot = make_rot((phys::real)M_PI / 4.0_r, phys::vec3(0.0_r, 1.0_r, 0.0_r));
						box_body_2.calculate_derived_data();
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::edge e(0, 4);
						phys::vclip::vertex v = p2.vertices[3];

						phys::vclip::algorithm_state state = phys::vclip::ve_state(
							p2,
							p1,
							v,
							e
						);

						expect(state).to_be(phys::vclip::algorithm_state{
							.f1 = v,
							.f2 = e,
							.step = phys::vclip::algorithm_step::Done,
							.penetration = 0.0_r
						});
					});
				});

				describe("V-F state handler", []() {
					after_each([&]() {
						box_body_1 = {};
						box_body_2 = {};
						box_1.half_size = phys::vec3(1.0_r);
						box_2.half_size = phys::vec3(1.0_r);
					});

					it("selects the closest edge when the vertex is excluded from the face's Voronoi region", [&]() {
						box_body_2.pos = phys::vec3(1.0_r, 2.5_r, 2.5_r);
						box_body_2.rot = make_rot((phys::real)M_PI / 4.0_r, phys::vec3(0.0_r, 1.0_r, 0.0_r));
						box_body_2.calculate_derived_data();
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::face f({ 0, 1, 5, 4 });
						phys::vclip::vertex v = p2.vertices[7];

						phys::vclip::algorithm_state state = phys::vclip::vf_state(
							p2,
							p1,
							v,
							f
						);

						expect(state).to_be(phys::vclip::algorithm_state{
							.f1 = v,
							.f2 = phys::vclip::edge(0, 4),
							.step = phys::vclip::algorithm_step::Continue,
							.penetration = 0.0_r
						});
					});

					it("selects the closest edge when the vertex is excluded from the face's Voronoi region (2)", [&]() {
						box_body_2.pos = phys::vec3(1.0_r, 2.5_r, -2.5_r);
						box_body_2.rot = make_rot((phys::real)M_PI / 4.0_r, phys::vec3(0.0_r, 1.0_r, 0.0_r));
						box_body_2.calculate_derived_data();
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::face f({ 0, 1, 5, 4 });
						phys::vclip::vertex v = p2.vertices[6];

						phys::vclip::algorithm_state state = phys::vclip::vf_state(
							p2,
							p1,
							v,
							f
						);

						expect(state).to_be(phys::vclip::algorithm_state{
							.f1 = v,
							.f2 = phys::vclip::edge(1, 5),
							.step = phys::vclip::algorithm_step::Continue,
							.penetration = 0.0_r
						});
					});

					it("selects the closest edge when the vertex is in the face's Voronoi region", [&]() {
						box_body_2.pos = phys::vec3(0.0_r, 2.5_r, 1.0_r);
						box_body_2.rot = make_rot((phys::real)M_PI / 16.0_r, phys::vec3(1.0_r, 0.0_r, 0.0_r)) *
							make_rot((phys::real)M_PI / 4.0_r, phys::vec3(0.0_r, 0.0_r, 1.0_r));
						box_body_2.calculate_derived_data();
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::face f({ 0, 1, 5, 4 });
						phys::vclip::vertex v = p2.vertices[7];

						phys::vclip::algorithm_state state = phys::vclip::vf_state(
							p2,
							p1,
							v,
							f
						);

						expect(state).to_be(phys::vclip::algorithm_state{
							.f1 = phys::vclip::edge(7, 6),
							.f2 = f,
							.step = phys::vclip::algorithm_step::Continue,
							.penetration = 0.0_r
						});
					});

					it("selects the closest edge when the vertex is in the face's Voronoi region "
						"and is incident to an edge that deeply penetrates the face", [&]() {
						box_body_2.pos = phys::vec3(0.0_r, 1.3_r, 1.0_r);
						box_body_2.rot = make_rot((phys::real)M_PI / 4.0_r, phys::vec3(1.0_r, 0.0_r, 0.0_r)) *
							make_rot((phys::real)M_PI / 4.0_r, phys::vec3(0.0_r, 0.0_r, 1.0_r));
						box_body_2.calculate_derived_data();
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::face f({ 0, 1, 5, 4 });
						phys::vclip::vertex v = p2.vertices[7];

						phys::vclip::algorithm_state state = phys::vclip::vf_state(
							p2,
							p1,
							v,
							f
						);

						expect(state).to_be(phys::vclip::algorithm_state{
							.f1 = phys::vclip::edge(7, 6),
							.f2 = f,
							.step = phys::vclip::algorithm_step::Continue,
							.penetration = 0.0_r
						});
					});

					it("terminates when the vertex is closest to the face", [&]() {
						box_body_2.pos = phys::vec3(0.0_r, 3.0_r, 1.0_r);
						box_body_2.rot = make_rot(-(phys::real)M_PI / 16.0_r, phys::vec3(1.0_r, 0.0_r, 0.0_r)) *
							make_rot((phys::real)M_PI / 4.0_r, phys::vec3(0.0_r, 0.0_r, 1.0_r));
						box_body_2.calculate_derived_data();
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::face f({ 0, 1, 5, 4 });
						phys::vclip::vertex v = p2.vertices[7];

						phys::vclip::algorithm_state state = phys::vclip::vf_state(
							p2,
							p1,
							v,
							f
						);

						expect(state).to_be(phys::vclip::algorithm_state{
							.f1 = v,
							.f2 = f,
							.step = phys::vclip::algorithm_step::Done,
							.penetration = 0.0_r
						});
					});

					it("reports penetration when the vertex shallowly penetrates the face", [&]() {
						box_body_2.pos = phys::vec3(0.0_r, 2.5_r, 1.0_r);
						box_body_2.rot = make_rot(-(phys::real)M_PI / 16.0_r, phys::vec3(1.0_r, 0.0_r, 0.0_r)) *
							make_rot((phys::real)M_PI / 4.0_r, phys::vec3(0.0_r, 0.0_r, 1.0_r));
						box_body_2.calculate_derived_data();
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::face f({ 0, 1, 5, 4 });
						phys::vclip::vertex v = p2.vertices[7];

						phys::vclip::algorithm_state state = phys::vclip::vf_state(
							p2,
							p1,
							v,
							f
						);

						expect(state.f1).to_be(v);
						expect(state.f2).to_be(f);
						expect(state.step).to_be(phys::vclip::algorithm_step::Penetration);
						expect(state.penetration).to_be_less_than(0.0_r);
					});

					it("selects a new face when the vertex is in a local minimum without penetration", [&]() {
						box_body_2.pos = phys::vec3(0.0_r, 3.0_r, 1.0_r);
						box_body_2.rot = make_rot(-(phys::real)M_PI / 16.0_r, phys::vec3(1.0_r, 0.0_r, 0.0_r)) *
							make_rot((phys::real)M_PI / 4.0_r, phys::vec3(0.0_r, 0.0_r, 1.0_r));
						box_body_2.calculate_derived_data();
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::face f({ 2, 6, 7, 3 });
						phys::vclip::vertex v = p2.vertices[7];

						phys::vclip::algorithm_state state = phys::vclip::vf_state(
							p2,
							p1,
							v,
							f
						);

						expect(state).to_be(phys::vclip::algorithm_state{
							.f1 = v,
							.f2 = phys::vclip::face({ 0, 1, 5, 4 }),
							.step = phys::vclip::algorithm_step::Continue,
							.penetration = 0.0_r
						});
					});

					it("reports penetration when the vertex is in a local minimum with penetration", [&]() {
						box_body_2.pos = phys::vec3(0.0_r, 0.7_r, 1.0_r);
						box_body_2.rot = make_rot(-(phys::real)M_PI / 16.0_r, phys::vec3(1.0_r, 0.0_r, 0.0_r)) *
							make_rot((phys::real)M_PI / 4.0_r, phys::vec3(0.0_r, 0.0_r, 1.0_r));
						box_body_2.calculate_derived_data();
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::face f({ 2, 6, 7, 3 });
						phys::vclip::vertex v = p2.vertices[7];

						phys::vclip::algorithm_state state = phys::vclip::vf_state(
							p2,
							p1,
							v,
							f
						);

						expect(state.f1).to_be(v);
						expect(state.f2).to_be(f);
						expect(state.step).to_be(phys::vclip::algorithm_step::Penetration);
						expect(state.penetration).to_be_less_than(0.0_r);
					});
				});

				describe("E-E state handler", []() {
					after_each([&]() {
						box_body_1 = {};
						box_body_2 = {};
						box_1.half_size = phys::vec3(1.0_r);
						box_2.half_size = phys::vec3(1.0_r);
					});

					it("terminates when the second edge intersects one F-E plane", [&]() {
						box_body_2.pos = phys::vec3(1.0_r, 1.0_r, 3.0_r);
						box_body_2.rot = make_rot((phys::real)M_PI / 4.0_r, phys::vec3(0.0_r, 1.0_r, 0.0_r));
						box_body_2.calculate_derived_data();
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::edge e1(4, 0);
						phys::vclip::edge e2(1, 3);

						phys::vclip::algorithm_state state = phys::vclip::ee_state(
							p1,
							p2,
							e1,
							e2
						);

						expect(state)
							.to_be(phys::vclip::algorithm_state{
								.f1 = e1,
								.f2 = e2,
								.step = phys::vclip::algorithm_step::Done,
								.penetration = 0.0_r
							});
					});

					it("terminates when the second edge intersects one F-E plane "
						"and the first edge is reversed", [&]() {
						box_body_2.pos = phys::vec3(1.0_r, 1.0_r, 3.0_r);
						box_body_2.rot = make_rot((phys::real)M_PI / 4.0_r, phys::vec3(0.0_r, 1.0_r, 0.0_r));
						box_body_2.calculate_derived_data();
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::edge e1(0, 4);
						phys::vclip::edge e2(1, 3);

						phys::vclip::algorithm_state state = phys::vclip::ee_state(
							p1,
							p2,
							e1,
							e2
						);

						expect(state)
							.to_be(phys::vclip::algorithm_state{
								.f1 = e1,
								.f2 = e2,
								.step = phys::vclip::algorithm_step::Done,
								.penetration = 0.0_r
							});
					});

					it("terminates when the second edge intersects one F-E plane "
						"and the second edge is reversed", [&]() {
						box_body_2.pos = phys::vec3(1.0_r, 1.0_r, 3.0_r);
						box_body_2.rot = make_rot((phys::real)M_PI / 4.0_r, phys::vec3(0.0_r, 1.0_r, 0.0_r));
						box_body_2.calculate_derived_data();
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::edge e1(4, 0);
						phys::vclip::edge e2(3, 1);

						phys::vclip::algorithm_state state = phys::vclip::ee_state(
							p1,
							p2,
							e1,
							e2
						);

						expect(state)
							.to_be(phys::vclip::algorithm_state{
								.f1 = e1,
								.f2 = e2,
								.step = phys::vclip::algorithm_step::Done,
								.penetration = 0.0_r
							});
					});

					it("terminates when the second edge intersects one F-E plane "
						"and both edges are reversed", [&]() {
						box_body_2.pos = phys::vec3(1.0_r, 1.0_r, 3.0_r);
						box_body_2.rot = make_rot((phys::real)M_PI / 4.0_r, phys::vec3(0.0_r, 1.0_r, 0.0_r));
						box_body_2.calculate_derived_data();
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::edge e1(0, 4);
						phys::vclip::edge e2(3, 1);

						phys::vclip::algorithm_state state = phys::vclip::ee_state(
							p1,
							p2,
							e1,
							e2
						);

						expect(state)
							.to_be(phys::vclip::algorithm_state{
								.f1 = e1,
								.f2 = e2,
								.step = phys::vclip::algorithm_step::Done,
								.penetration = 0.0_r
							});
					});

					it("terminates when the second edge intersects one F-E plane "
						"and the edges are swapped", [&]() {
						box_body_2.pos = phys::vec3(1.0_r, 1.0_r, 3.0_r);
						box_body_2.rot = make_rot((phys::real)M_PI / 4.0_r, phys::vec3(0.0_r, 1.0_r, 0.0_r));
						box_body_2.calculate_derived_data();
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::edge e1(4, 0);
						phys::vclip::edge e2(1, 3);

						phys::vclip::algorithm_state state = phys::vclip::ee_state(
							p2,
							p1,
							e2,
							e1
						);

						expect(state)
							.to_be(phys::vclip::algorithm_state{
								.f1 = e2,
								.f2 = e1,
								.step = phys::vclip::algorithm_step::Done,
								.penetration = 0.0_r
							});
					});

					it("terminates when the second edge intersects one F-E plane", [&]() {
						box_body_2.pos = phys::vec3(1.0_r, 1.0_r, 3.0_r);
						box_body_2.calculate_derived_data();
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::edge e1(4, 0);
						phys::vclip::edge e2(5, 7);

						phys::vclip::algorithm_state state = phys::vclip::ee_state(
							p1,
							p2,
							e1,
							e2
						);

						expect(state).to_be(phys::vclip::algorithm_state{
							.f1 = e1,
							.f2 = e2,
							.step = phys::vclip::algorithm_step::Done,
							.penetration = 0.0_r
						});
					});

					it("selects the closest vertex when the second edge intersects one V-E plane", [&]() {
						box_body_2.pos = phys::vec3(2.5_r, 1.0_r, 2.5_r);
						box_body_2.calculate_derived_data();
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::edge e1(4, 0);
						phys::vclip::edge e2(5, 7);

						phys::vclip::algorithm_state state = phys::vclip::ee_state(
							p1,
							p2,
							e1,
							e2
						);

						expect(state).to_be(phys::vclip::algorithm_state{
							.f1 = p1.vertices[0],
							.f2 = e2,
							.step = phys::vclip::algorithm_step::Continue,
							.penetration = 0.0_r
						});
					});

					it("selects the closest vertex when the second edge intersects one V-E plane "
						"and the first edge is reversed", [&]() {
						box_body_2.pos = phys::vec3(2.5_r, 1.0_r, 2.5_r);
						box_body_2.calculate_derived_data();
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::edge e1(0, 4);
						phys::vclip::edge e2(5, 7);

						phys::vclip::algorithm_state state = phys::vclip::ee_state(
							p1,
							p2,
							e1,
							e2
						);

						expect(state).to_be(phys::vclip::algorithm_state{
							.f1 = p1.vertices[0],
							.f2 = e2,
							.step = phys::vclip::algorithm_step::Continue,
							.penetration = 0.0_r
						});
					});

					it("selects the closest vertex when the second edge intersects one V-E plane "
						"and the second edge is reversed", [&]() {
						box_body_2.pos = phys::vec3(2.5_r, 1.0_r, 2.5_r);
						box_body_2.calculate_derived_data();
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::edge e1(4, 0);
						phys::vclip::edge e2(7, 5);

						phys::vclip::algorithm_state state = phys::vclip::ee_state(
							p1,
							p2,
							e1,
							e2
						);

						expect(state).to_be(phys::vclip::algorithm_state{
							.f1 = p1.vertices[0],
							.f2 = e2,
							.step = phys::vclip::algorithm_step::Continue,
							.penetration = 0.0_r
						});
					});

					it("selects the closest vertex when the second edge intersects one V-E plane "
						"and both edges are reversed", [&]() {
						box_body_2.pos = phys::vec3(2.5_r, 1.0_r, 2.5_r);
						box_body_2.calculate_derived_data();
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::edge e1(0, 4);
						phys::vclip::edge e2(7, 5);

						phys::vclip::algorithm_state state = phys::vclip::ee_state(
							p1,
							p2,
							e1,
							e2
						);

						expect(state).to_be(phys::vclip::algorithm_state{
							.f1 = p1.vertices[0],
							.f2 = e2,
							.step = phys::vclip::algorithm_step::Continue,
							.penetration = 0.0_r
						});
					});

					it("selects the closest vertex when the second edge intersects one V-E plane "
						"and the edges are swapped", [&]() {
						box_body_2.pos = phys::vec3(2.5_r, 1.0_r, 2.5_r);
						box_body_2.calculate_derived_data();
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::edge e1(4, 0);
						phys::vclip::edge e2(5, 7);

						phys::vclip::algorithm_state state = phys::vclip::ee_state(
							p2,
							p1,
							e2,
							e1
						);

						expect(state).to_be(phys::vclip::algorithm_state{
							.f1 = e2,
							.f2 = p1.vertices[0],
							.step = phys::vclip::algorithm_step::Continue,
							.penetration = 0.0_r
						});
					});
				});

				describe("E-F state handler", []() {
					after_each([&]() {
						box_body_1 = {};
						box_body_2 = {};
						box_1.half_size = phys::vec3(1.0_r);
						box_2.half_size = phys::vec3(1.0_r);
					});

					it("selects the closer edge for an edge clipped on both sides by a face", [&]() {
						box_body_2.pos = phys::vec3(1.0_r, 3.5_r, 0.0_r);
						box_body_2.rot = make_rot((phys::real)M_PI / 16.0_r, phys::vec3(1.0_r, 0.0_r, 0.0_r));
						box_body_2.calculate_derived_data();
						box_2.half_size = phys::vec3(2.0_r);
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::face f({ 0, 1, 5, 4 });
						phys::vclip::edge e(7, 6);

						phys::vclip::algorithm_state state = phys::vclip::ef_state(
							p2,
							p1,
							e,
							f
						);

						expect(state).to_be(phys::vclip::algorithm_state{
							.f1 = e,
							.f2 = phys::vclip::edge(4, 0),
							.step = phys::vclip::algorithm_step::Continue,
							.penetration = 0.0_r
						});
					});

					it("selects the closer edge for an edge clipped on both sides by a face "
						"when the clipped edge is reversed", [&]() {
						box_body_2.pos = phys::vec3(1.0_r, 3.5_r, 0.0_r);
						box_body_2.rot = make_rot((phys::real)M_PI / 16.0_r, phys::vec3(1.0_r, 0.0_r, 0.0_r));
						box_body_2.calculate_derived_data();
						box_2.half_size = phys::vec3(2.0_r);
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::face f({ 0, 1, 5, 4 });
						phys::vclip::edge e(6, 7);

						phys::vclip::algorithm_state state = phys::vclip::ef_state(
							p2,
							p1,
							e,
							f
						);

						expect(state).to_be(phys::vclip::algorithm_state{
							.f1 = e,
							.f2 = phys::vclip::edge(4, 0),
							.step = phys::vclip::algorithm_step::Continue,
							.penetration = 0.0_r
						});
					});

					it("selects the closer edge for an edge clipped on both sides by a face (2)", [&]() {
						box_body_2.pos = phys::vec3(1.0_r, 3.5_r, 0.0_r);
						box_body_2.rot = make_rot(-(phys::real)M_PI / 16.0_r, phys::vec3(1.0_r, 0.0_r, 0.0_r));
						box_body_2.calculate_derived_data();
						box_2.half_size = phys::vec3(2.0_r);
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::face f({ 0, 1, 5, 4 });
						phys::vclip::edge e(7, 6);

						phys::vclip::algorithm_state state = phys::vclip::ef_state(
							p2,
							p1,
							e,
							f
						);

						expect(state).to_be(phys::vclip::algorithm_state{
							.f1 = e,
							.f2 = phys::vclip::edge(1, 5),
							.step = phys::vclip::algorithm_step::Continue,
							.penetration = 0.0_r
						});
					});

					it("selects the closer edge for an edge clipped on both sides by a face "
						"when the clipped edge is reversed (2)", [&]() {
						box_body_2.pos = phys::vec3(1.0_r, 3.5_r, 0.0_r);
						box_body_2.rot = make_rot(-(phys::real)M_PI / 16.0_r, phys::vec3(1.0_r, 0.0_r, 0.0_r));
						box_body_2.calculate_derived_data();
						box_2.half_size = phys::vec3(2.0_r);
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::face f({ 0, 1, 5, 4 });
						phys::vclip::edge e(6, 7);

						phys::vclip::algorithm_state state = phys::vclip::ef_state(
							p2,
							p1,
							e,
							f
						);

						expect(state).to_be(phys::vclip::algorithm_state{
							.f1 = e,
							.f2 = phys::vclip::edge(1, 5),
							.step = phys::vclip::algorithm_step::Continue,
							.penetration = 0.0_r
						});
					});

					it("does not terminate for an edge clipped on both sides and closest to the face", [&]() {
						box_body_2.pos = phys::vec3(1.0_r, 3.5_r, 0.0_r);
						box_body_2.calculate_derived_data();
						box_2.half_size = phys::vec3(2.0_r);
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::face f({ 0, 1, 5, 4 });
						phys::vclip::edge e(7, 6);

						phys::vclip::algorithm_state state = phys::vclip::ef_state(
							p2,
							p1,
							e,
							f
						);

						expect(state)
							.to_be(phys::vclip::algorithm_state{
								.f1 = e,
								.f2 = phys::vclip::edge(1, 5),
								.step = phys::vclip::algorithm_step::Continue,
								.penetration = 0.0_r
							}).orr()
							.to_be(phys::vclip::algorithm_state{
								.f1 = e,
								.f2 = phys::vclip::edge(0, 4),
								.step = phys::vclip::algorithm_step::Continue,
								.penetration = 0.0_r
							});
					});

					it("does not terminate for an edge clipped on both sides and closest to the face "
						"when the clipped edge is reversed", [&]() {
						box_body_2.pos = phys::vec3(1.0_r, 3.5_r, 0.0_r);
						box_body_2.calculate_derived_data();
						box_2.half_size = phys::vec3(2.0_r);
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::face f({ 0, 1, 5, 4 });
						phys::vclip::edge e(6, 7);

						phys::vclip::algorithm_state state = phys::vclip::ef_state(
							p2,
							p1,
							e,
							f
						);

						expect(state)
							.to_be(phys::vclip::algorithm_state{
								.f1 = e,
								.f2 = phys::vclip::edge(1, 5),
								.step = phys::vclip::algorithm_step::Continue,
								.penetration = 0.0_r
							}).orr()
							.to_be(phys::vclip::algorithm_state{
								.f1 = e,
								.f2 = phys::vclip::edge(0, 4),
								.step = phys::vclip::algorithm_step::Continue,
								.penetration = 0.0_r
							});
					});

					it("terminates for an edge clipped on both sides and penetrating the face", [&]() {
						box_body_2.pos = phys::vec3(1.0_r, 3.0_r, 0.0_r);
						box_body_2.rot = make_rot((phys::real)M_PI / 16.0_r, phys::vec3(1.0_r, 0.0_r, 0.0_r));
						box_body_2.calculate_derived_data();
						box_2.half_size = phys::vec3(2.0_r);
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::face f({ 0, 1, 5, 4 });
						phys::vclip::edge e(7, 6);

						phys::vclip::algorithm_state state = phys::vclip::ef_state(
							p2,
							p1,
							e,
							f
						);

						expect(state.f1).to_be(e);
						expect(state.f2).to_be(f);
						expect(state.step).to_be(phys::vclip::algorithm_step::Penetration);
						expect(state.penetration).to_be_less_than(0.0_r);
					});

					it("terminates for an edge clipped on both sides and penetrating the face "
						"when the clipped edge is reversed", [&]() {
						box_body_2.pos = phys::vec3(1.0_r, 3.0_r, 0.0_r);
						box_body_2.rot = make_rot((phys::real)M_PI / 16.0_r, phys::vec3(1.0_r, 0.0_r, 0.0_r));
						box_body_2.calculate_derived_data();
						box_2.half_size = phys::vec3(2.0_r);
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::face f({ 0, 1, 5, 4 });
						phys::vclip::edge e(6, 7);

						phys::vclip::algorithm_state state = phys::vclip::ef_state(
							p2,
							p1,
							e,
							f
						);

						expect(state.f1).to_be(e);
						expect(state.f2).to_be(f);
						expect(state.step).to_be(phys::vclip::algorithm_step::Penetration);
						expect(state.penetration).to_be_less_than(0.0_r);
					});

					it("selects the closest vertex for an edge penetrating one of a face's F-E planes", [&]() {
						box_body_2.pos = phys::vec3(1.0_r, 2.5_r, 0.0_r);
						box_body_2.rot = make_rot((phys::real)M_PI / 4.0_r, phys::vec3(1.0_r, 0.0_r, 0.0_r));
						box_body_2.calculate_derived_data();
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::face f({ 0, 1, 5, 4 });
						phys::vclip::edge e(7, 6);

						phys::vclip::algorithm_state state = phys::vclip::ef_state(
							p2,
							p1,
							e,
							f
						);

						expect(state).to_be(phys::vclip::algorithm_state{
							.f1 = p2.vertices[6],
							.f2 = f,
							.step = phys::vclip::algorithm_step::Continue,
							.penetration = 0.0_r
						});
					});

					it("selects the closest edge for an edge penetrating one of a face's F-E planes", [&]() {
						box_body_2.pos = phys::vec3(1.0_r, 2.5_r, 1.0_r);
						box_body_2.rot = make_rot((phys::real)M_PI / 16.0_r, phys::vec3(1.0_r, 0.0_r, 0.0_r));
						box_body_2.calculate_derived_data();
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::face f({ 0, 1, 5, 4 });
						phys::vclip::edge e(7, 6);

						phys::vclip::algorithm_state state = phys::vclip::ef_state(
							p2,
							p1,
							e,
							f
						);

						expect(state).to_be(phys::vclip::algorithm_state{
							.f1 = e,
							.f2 = phys::vclip::edge(0, 4),
							.step = phys::vclip::algorithm_step::Continue,
							.penetration = 0.0_r
						});
					});

					it("reports penetration for an edge penetrating one of a face's F-E planes", [&]() {
						box_body_2.pos = phys::vec3(1.0_r, 1.5_r, 1.0_r);
						box_body_2.calculate_derived_data();
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::face f({ 0, 1, 5, 4 });
						phys::vclip::edge e(5, 7);

						phys::vclip::algorithm_state state = phys::vclip::ef_state(
							p2,
							p1,
							e,
							f
						);

						expect(state).to_be(phys::vclip::algorithm_state{
							.f1 = e,
							.f2 = f,
							.step = phys::vclip::algorithm_step::Penetration,
							.penetration = -0.5_r
						});
					});

					it("does not report penetration for support plane violation outside of the plane's Voronoi region", [&]() {
						box_body_2.pos = phys::vec3(1.0_r, 2.5_r, 1.0_r);
						box_body_2.rot = make_rot((phys::real)M_PI / 4.0_r, phys::vec3(1.0_r, 0.0_r, 0.0_r));
						box_body_2.calculate_derived_data();
						box_2.half_size = phys::vec3(1.0_r, 1.0_r, 5.0_r);
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::face f({ 0, 1, 5, 4 });
						phys::vclip::edge e(7, 6);

						phys::vclip::algorithm_state state = phys::vclip::ef_state(
							p2,
							p1,
							e,
							f
						);

						expect(state).to_be(phys::vclip::algorithm_state{
							.f1 = e,
							.f2 = phys::vclip::edge(0, 4),
							.step = phys::vclip::algorithm_step::Continue,
							.penetration = 0.0_r
						});
					});

					it("selects the closest edge on the face when the edge is simply excluded from the face", [&]() {
						box_body_2.pos = phys::vec3(1.0_r, 1.0_r, 2.5_r);
						box_body_2.calculate_derived_data();
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::face f({ 0, 1, 5, 4 });
						phys::vclip::edge e(7, 3);

						phys::vclip::algorithm_state state = phys::vclip::ef_state(
							p2,
							p1,
							e,
							f
						);

						expect(state).to_be(phys::vclip::algorithm_state{
							.f1 = e,
							.f2 = phys::vclip::edge(0, 4),
							.step = phys::vclip::algorithm_step::Continue,
							.penetration = 0.0_r
						});
					});

					it("selects the closest edge on the face when the edge is simply excluded from the face (2)", [&]() {
						box_body_2.pos = phys::vec3(1.0_r, 1.0_r, -1.5_r);
						box_body_2.calculate_derived_data();
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::face f({ 0, 1, 5, 4 });
						phys::vclip::edge e(7, 3);

						phys::vclip::algorithm_state state = phys::vclip::ef_state(
							p2,
							p1,
							e,
							f
						);

						expect(state).to_be(phys::vclip::algorithm_state{
							.f1 = e,
							.f2 = phys::vclip::edge(1, 5),
							.step = phys::vclip::algorithm_step::Continue,
							.penetration = 0.0_r
						});
					});

					it("selects the closest edge on the face when the edge is excluded (compound) from the face", [&]() {
						box_body_2.pos = phys::vec3(2.5_r);
						box_body_2.rot = make_rot((phys::real)M_PI / 4.0_r, phys::vec3(0.0_r, 1.0_r, 0.0_r));
						box_body_2.calculate_derived_data();
						box_2.half_size = phys::vec3(2.0_r, 1.0_r, 1.0_r);
						phys::vclip::polyhedron p1 = box_1.to_polyhedron();
						phys::vclip::polyhedron p2 = box_2.to_polyhedron();
						phys::vclip::face f({ 0, 1, 5, 4 });
						phys::vclip::edge e(3, 7);

						phys::vclip::algorithm_state state = phys::vclip::ef_state(
							p2,
							p1,
							e,
							f
						);

						expect(state).to_be(phys::vclip::algorithm_state{
							.f1 = e,
							.f2 = phys::vclip::edge(0, 1),
							.step = phys::vclip::algorithm_step::Continue,
							.penetration = 0.0_r
						}).orr().to_be(phys::vclip::algorithm_state{
							.f1 = e,
							.f2 = phys::vclip::edge(4, 0),
							.step = phys::vclip::algorithm_step::Continue,
							.penetration = 0.0_r
						});
					});
				});
			});

			describe("collision detection", []() {
				after_each([&]() {
					box_body_1 = {};
					box_body_2 = {};
					box_1.half_size = phys::vec3(1.0_r);
					box_2.half_size = phys::vec3(1.0_r);
				});

				it("always reports penetration", [&]() {
					box_body_2.pos = phys::vec3(0.0_r, 2.0_r, 1.0_r);
					box_body_2.rot = make_rot(-(phys::real)M_PI / 16.0_r, phys::vec3(1.0_r, 0.0_r, 0.0_r)) *
						make_rot((phys::real)M_PI / 4.0_r, phys::vec3(0.0_r, 0.0_r, 1.0_r));
					box_body_2.calculate_derived_data();
					box_body_1.calculate_derived_data();
					phys::vclip::polyhedron p1 = box_1.to_polyhedron();
					phys::vclip::polyhedron p2 = box_2.to_polyhedron();

					p1.validate();
					p2.validate();

					for (const phys::vclip::feature &f1 : p1.features()) {
						for (const phys::vclip::feature &f2 : p2.features()) {
							if (std::holds_alternative<phys::vclip::face>(f1) && std::holds_alternative<phys::vclip::face>(f2)) {
								continue;
							}

							phys::vclip::algorithm_result result = phys::vclip::closest_features(
								p1,
								p2,
								f1,
								f2,
								52
							);

							expect(result.state)
								.to_be(phys::vclip::algorithm_state{
									.f1 = phys::vclip::face({ 0, 4, 6, 2 }),
									.f2 = phys::vclip::edge(6, 7),
									.step = phys::vclip::algorithm_step::Penetration,
									.penetration = -0.704886_r
								}).orr()
								.to_be(phys::vclip::algorithm_state{
									.f1 = phys::vclip::face({ 0, 4, 6, 2 }),
									.f2 = p2.vertices[7],
									.step = phys::vclip::algorithm_step::Penetration,
									.penetration = -0.582130_r
								}).orr()
								.to_be(phys::vclip::algorithm_state{
									.f1 = phys::vclip::face({ 0, 1, 5, 4 }),
									.f2 = p2.vertices[7],
									.step = phys::vclip::algorithm_step::Penetration,
									.penetration = -0.582130_r
								}).orr()
								.to_be(phys::vclip::algorithm_state{
									.f1 = phys::vclip::face({ 0, 1, 5, 4 }),
									.f2 = phys::vclip::edge(3, 7),
									.step = phys::vclip::algorithm_step::Penetration,
									.penetration = -0.582130_r
								}).orr()
								.to_be(phys::vclip::algorithm_state{
									.f1 = phys::vclip::face({ 0, 1, 5, 4 }),
									.f2 = phys::vclip::edge(5, 7),
									.step = phys::vclip::algorithm_step::Penetration,
									.penetration = -0.582130_r
								}).orr()
								.to_be(phys::vclip::algorithm_state{
									.f1 = phys::vclip::edge(0, 4),
									.f2 = phys::vclip::face({ 4, 5, 7, 6 }),
									.step = phys::vclip::algorithm_step::Penetration,
									.penetration = -0.612960_r
								}).orr()
								.to_be(phys::vclip::algorithm_state{
									.f1 = phys::vclip::edge(0, 4),
									.f2 = phys::vclip::face({ 2, 6, 7, 3 }),
									.step = phys::vclip::algorithm_step::Penetration,
									.penetration = -0.612960_r
								});
						}
					}
				});

				it("always reports penetration (2)", [&]() {
					box_body_2.pos = phys::vec3(0.0_r, 2.0_r, 1.0_r);
					box_body_2.rot = make_rot(-(phys::real)M_PI / 20.0_r, phys::vec3(0.0_r, 1.0_r, 0.0_r)) *
						make_rot(-(phys::real)M_PI / 16.0_r, phys::vec3(1.0_r, 0.0_r, 0.0_r)) *
						make_rot((phys::real)M_PI / 4.0_r, phys::vec3(0.0_r, 0.0_r, 1.0_r));
					box_body_2.calculate_derived_data();
					box_body_1.calculate_derived_data();
					phys::vclip::polyhedron p1 = box_1.to_polyhedron();
					phys::vclip::polyhedron p2 = box_2.to_polyhedron();

					p1.validate();
					p2.validate();

					for (const phys::vclip::feature &f1 : p1.features()) {
						for (const phys::vclip::feature &f2 : p2.features()) {
							if (
								std::holds_alternative<phys::vclip::face>(f1) &&
								std::holds_alternative<phys::vclip::face>(f2)
							) {
								continue;
							}

							phys::vclip::algorithm_result result = phys::vclip::closest_features(
								p1,
								p2,
								f1,
								f2,
								52
							);

							expect(result.state)
								.to_be(phys::vclip::algorithm_state{
									.f1 = phys::vclip::face({ 0, 4, 6, 2 }),
									.f2 = phys::vclip::edge(6, 7),
									.step = phys::vclip::algorithm_step::Penetration,
									.penetration = -0.696208_r
								}).orr()
								.to_be(phys::vclip::algorithm_state{
									.f1 = phys::vclip::face({ 0, 4, 6, 2 }),
									.f2 = p2.vertices[7],
									.step = phys::vclip::algorithm_step::Penetration,
									.penetration = -0.582130_r
								}).orr()
								.to_be(phys::vclip::algorithm_state{
									.f1 = phys::vclip::face({ 0, 1, 5, 4 }),
									.f2 = p2.vertices[7],
									.step = phys::vclip::algorithm_step::Penetration,
									.penetration = -0.582130_r
								}).orr()
								.to_be(phys::vclip::algorithm_state{
									.f1 = phys::vclip::face({ 0, 1, 5, 4 }),
									.f2 = phys::vclip::edge(3, 7),
									.step = phys::vclip::algorithm_step::Penetration,
									.penetration = -0.582130_r
								}).orr()
								.to_be(phys::vclip::algorithm_state{
									.f1 = phys::vclip::face({ 0, 1, 5, 4 }),
									.f2 = phys::vclip::edge(5, 7),
									.step = phys::vclip::algorithm_step::Penetration,
									.penetration = -0.582130_r
								}).orr()
								.to_be(phys::vclip::algorithm_state{
									.f1 = phys::vclip::edge(0, 4),
									.f2 = phys::vclip::face({ 4, 5, 7, 6 }),
									.step = phys::vclip::algorithm_step::Penetration,
									.penetration = -0.632504_r
								}).orr()
								.to_be(phys::vclip::algorithm_state{
									.f1 = phys::vclip::edge(0, 4),
									.f2 = phys::vclip::face({ 2, 6, 7, 3 }),
									.step = phys::vclip::algorithm_step::Penetration,
									.penetration = -0.594588_r
								});
						}
					}
				});

				it("always terminates when the boxes are not penetrating", [&]() {
					box_body_2.pos = phys::vec3(-1.0_r, -4.0_r, 0.0_r);

					box_body_2.calculate_derived_data();

					phys::vclip::polyhedron p1 = box_1.to_polyhedron();
					phys::vclip::polyhedron p2 = box_2.to_polyhedron();

					p1.validate();
					p2.validate();

					for (const phys::vclip::feature &f1 : p1.features()) {
						for (const phys::vclip::feature &f2 : p2.features()) {
							if (
								std::holds_alternative<phys::vclip::face>(f1) &&
								std::holds_alternative<phys::vclip::face>(f2)
							) {
								continue;
							}

							phys::vclip::algorithm_result result = phys::vclip::closest_features(
								p1,
								p2,
								f1,
								f2,
								52
							);

							expect(result.state.step).to_be(phys::vclip::algorithm_step::Done);
						}
					}
				});
			});
		});
	});
}