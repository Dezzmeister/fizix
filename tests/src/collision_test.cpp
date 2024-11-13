#include "physics/collision/contact_generator.h"
#include "physics/collision/primitives.h"
#include "test.h"

using namespace test;
using namespace phys::literals;

const phys::contact_generator collider{};
std::vector<phys::contact> contacts{};
phys::rigid_body sphere_body_1{};
phys::rigid_body sphere_body_2{};

void setup_collision_tests() {
	describe("Rigid body collisions", []() {
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

		it("detects two interpenetrating spheres when the args are switched", [&]() {
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
}