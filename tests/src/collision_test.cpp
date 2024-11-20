#define _USE_MATH_DEFINES
#include <math.h>
#include "physics/collision/contact_generator.h"
#include "physics/collision/primitives.h"
#include "test.h"

using namespace test;
using namespace phys::literals;

const phys::contact_generator collider{};
std::vector<phys::contact> contacts{};
phys::rigid_body sphere_body_1{};
phys::rigid_body sphere_body_2{};
phys::rigid_body box_body_1{};

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
	});
}