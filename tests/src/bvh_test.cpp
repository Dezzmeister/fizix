#define DEBUG
#include <random>
#include <stack>
#include "physics/collision/bvh.h"
#include "test.h"

using namespace test;

using sphere_bvh = phys::bvh<phys::bounding_sphere, int>;

namespace {
	using namespace phys::literals;

	phys::bounding_sphere s1(phys::vec3(2.0_r), 1.0_r);
	phys::bounding_sphere s2(phys::vec3(-2.0_r), 1.0_r);
	phys::bounding_sphere s3(phys::vec3(0.5_r), 0.25_r);
	phys::bounding_sphere s4(phys::vec3(-0.5_r), -0.25_r);

	template <typename BVH>
	void bvh_checks(const BVH &objects) {
		using node_t = typename BVH::node;

		if (! objects.root.get()) {
			return;
		}

		std::stack<node_t *> nodes{};
		size_t obj_count = 0;

		nodes.push(objects.root.get());

		while (! nodes.empty()) {
			node_t * n = nodes.top();
			nodes.pop();

			node_t * left = n->left.get();
			node_t * right = n->right.get();

			if (!! left != !! right) {
				fail("expected node with id " + std::to_string(n->id) + " to have two children or none at all");
			}

			if (left) {
				if (n->id) {
					fail("expected internal node with id " + std::to_string(n->id) + " to have id 0");
				}

				nodes.push(left);
				nodes.push(right);
			} else {
				if (! n->id) {
					fail("expected leaf node to have nonzero id");
				}

				obj_count++;
			}

			if (n == objects.root.get()) {
				expect(n->parent).to_be(nullptr);
			} else {
				if (! n->parent) {
					fail("expected non-root node with id " + std::to_string(n->id) + " to have a parent");
				}

				node_t * p = n->parent;

				if (p->left.get() != n && p->right.get() != n) {
					fail("expected n->parent with id " + std::to_string(p->id) + " to have child n with id " + std::to_string(n->id));
				}
			}
		}

		expect(obj_count).to_be(objects.size());

		if (objects.ids.size() <= 1) {
			return;
		}

		for (size_t i = 1; i < objects.ids.size(); i++) {
			auto curr_id = objects.ids[i].id;
			auto prev_id = objects.ids[i - 1].id;

			if (curr_id < prev_id) {
				fail("expected ids " + std::to_string(prev_id) + " and " + std::to_string(curr_id) + " to be sorted in ascending order");
			}
		}
	}

	template <phys::bounding_volume Volume, typename Identifier>
	void volume_check(const phys::bvh<Volume, Identifier> &objects) {
		using node_t = typename phys::bvh<Volume, Identifier>::node;

		if (! objects.root.get()) {
			return;
		}

		std::stack<node_t *> nodes{};
		nodes.push(objects.root.get());

		while (! nodes.empty()) {
			node_t * n = nodes.top();
			nodes.pop();

			node_t * left = n->left.get();
			node_t * right = n->right.get();

			if (! left || ! right) {
				continue;
			}

			if (n->vol != Volume(left->vol, right->vol)) {
				fail("expected parent of nodes with ids " + std::to_string(left->id) + " and " + std::to_string(right->id) +
					" to have volume enclosing its children");
			}

			nodes.push(left);
			nodes.push(right);
		}
	}

	phys::bounding_sphere random_sphere(
		std::uniform_real_distribution<phys::real> &coord_distrib,
		std::uniform_real_distribution<phys::real> &radius_distrib
	) {
		static std::random_device rand{};

		phys::real x = coord_distrib(rand);
		phys::real y = coord_distrib(rand);
		phys::real z = coord_distrib(rand);
		phys::real r = radius_distrib(rand);

		return phys::bounding_sphere(phys::vec3(x, y, z), r);
	}

	template <typename Container>
	bool contains_collision(const Container &c, int id1, int id2) {
		for (const auto &pair : c) {
			if (pair.id1 == id1 && pair.id2 == id2 ||
				pair.id1 == id2 && pair.id2 == id1) {
				return true;
			}
		}

		return false;
	}
}

void setup_bvh_tests() {
	describe("BVH", []() {
		describe("with a spherical volume", []() {
			it("inserts and groups objects", []() {
				sphere_bvh objects{};

				objects.insert(1, s1);
				objects.insert(2, s2);
				objects.insert(3, s3);
				objects.insert(4, s4);

				expect(objects.size()).to_be(4);

				expect(objects.root->id).to_be(0);
				expect(objects.root->left->id).to_be(0);
				expect(objects.root->right->id).to_be(0);

				expect(objects.root->left->left->id).to_be(1);
				expect(objects.root->left->left->vol).to_be(s1);

				expect(objects.root->left->right->id).to_be(3);
				expect(objects.root->left->right->vol).to_be(s3);

				expect(objects.root->right->left->id).to_be(2);
				expect(objects.root->right->left->vol).to_be(s2);

				expect(objects.root->right->right->id).to_be(4);
				expect(objects.root->right->right->vol).to_be(s4);

				bvh_checks(objects);
			});

			it("deletes objects", []() {
				sphere_bvh objects{};

				objects.insert(1, s1);
				objects.insert(2, s2);
				objects.insert(3, s3);
				objects.insert(4, s4);

				expect(objects.remove(2)).to_be(true);
				expect(objects.size()).to_be(3);

				expect(objects.remove(4)).to_be(true);
				expect(objects.size()).to_be(2);

				expect(objects.remove(1)).to_be(true);
				expect(objects.size()).to_be(1);

				expect(objects.remove(3)).to_be(true);
				expect(objects.size()).to_be(0);
			});

			it("size does not underflow", []() {
				sphere_bvh objects{};

				objects.insert(1, s1);
				objects.insert(2, s2);
				objects.insert(3, s3);
				objects.insert(4, s4);

				expect(objects.size()).to_be(4);

				objects.remove(1);
				objects.remove(2);
				objects.remove(3);
				objects.remove(4);

				expect(objects.size()).to_be(0);

				for (int i = 0; i < 10; i++) {
					expect(objects.remove(i)).to_be(false);
				}

				expect(objects.size()).to_be(0);
			});

			it("inserts and deletes many objects", []() {
				std::uniform_real_distribution<phys::real> coord_distrib(-100.0_r, 100.0_r);
				std::uniform_real_distribution<phys::real> radius_distrib(0.1_r, 2.0_r);
				sphere_bvh objects{};

				for (int i = 1; i < 2000; i++) {
					objects.insert(i, random_sphere(coord_distrib, radius_distrib));
				}

				bvh_checks(objects);
				volume_check(objects);

				for (int i = 1999; i > 1000; i--) {
					if (! objects.remove(i)) {
						fail("object " + std::to_string(i) + " was not present");
					}
				}

				bvh_checks(objects);
				volume_check(objects);

				for (int i = 3000; i > 2000; i--) {
					objects.insert(i, random_sphere(coord_distrib, radius_distrib));
				}

				bvh_checks(objects);
				volume_check(objects);

				for (int i = 1000; i > 0; i--) {
					if (! objects.remove(i)) {
						fail("object " + std::to_string(i) + " was not present");
					}
				}

				bvh_checks(objects);
				volume_check(objects);

				for (int i = 3000; i > 2000; i--) {
					if (! objects.remove(i)) {
						fail("object " + std::to_string(i) + " was not present");
					}
				}

				bvh_checks(objects);
				volume_check(objects);
			});

			it("updates objects", []() {
				sphere_bvh objects{};

				objects.insert(1, s1);
				objects.insert(2, s2);
				objects.insert(4, s4);

				expect(objects.size()).to_be(3);

				expect(objects.root->id).to_be(0);
				expect(objects.root->left->vol).to_be(s1);
				expect(objects.root->right->id).to_be(0);
				expect(objects.root->right->left->vol).to_be(s2);
				expect(objects.root->right->right->vol).to_be(s4);

				objects.update(2, s3);

				expect(objects.size()).to_be(3);

				bvh_checks(objects);
				volume_check(objects);

				expect(objects.root->id).to_be(0);
				expect(objects.root->left->vol).to_be(s1);
				expect(objects.root->right->id).to_be(0);
				expect(objects.root->right->left->vol).to_be(s4);
				expect(objects.root->right->right->vol).to_be(s3);
				expect(objects.root->right->right->id).to_be(2);
			});

			it("does not delete objects that don't exist", []() {
				sphere_bvh objects{};

				objects.insert(1, s1);
				objects.insert(2, s2);
				objects.insert(4, s4);

				expect(objects.size()).to_be(3);

				expect(objects.remove(3)).to_be(false);
				expect(objects.size()).to_be(3);
			});

			it("indicates whether objects are present or not", []() {
				sphere_bvh objects{};

				objects.insert(1, s1);
				objects.insert(2, s2);
				objects.insert(4, s4);

				expect(objects.size()).to_be(3);
				expect(objects.has(1)).to_be(true);
				expect(objects.has(2)).to_be(true);
				expect(objects.has(4)).to_be(true);
				expect(objects.has(3)).to_be(false);
				expect(objects.has(1000)).to_be(false);
			});

			it("generates coarse collision pairs", []() {
				phys::bounding_sphere a1(phys::vec3(5.0_r), 1.0_r);
				phys::bounding_sphere a2(phys::vec3(4.0_r), 2.0_r);
				phys::bounding_sphere a3(phys::vec3(5.0_r, 4.0_r, 5.0_r), 1.0_r);

				phys::bounding_sphere b1(phys::vec3(-5.0_r), 1.0_r);
				phys::bounding_sphere b2(phys::vec3(-4.0_r), 2.0_r);
				phys::bounding_sphere b3(phys::vec3(-5.0_r, -4.0_r, -5.0_r), 1.0_r);

				phys::bounding_sphere c1(phys::vec3(10.0_r), 0.1_r);
				phys::bounding_sphere c2(phys::vec3(11.0_r), 0.2_r);
				phys::bounding_sphere c3(phys::vec3(12.0_r), 0.3_r);
				phys::bounding_sphere c4(phys::vec3(13.0_r), 0.4_r);

				sphere_bvh objects{};

				objects.insert(1, a1);
				objects.insert(11, b1);
				objects.insert(101, c1);
				objects.insert(2, a2);
				objects.insert(3, a3);
				objects.insert(102, c2);
				objects.insert(103, c3);
				objects.insert(104, c4);
				objects.insert(12, b2);
				objects.insert(13, b3);

				std::vector<sphere_bvh::coarse_collision_pair> collision_pairs{};

				objects.generate_coarse_collisions(collision_pairs);

				expect(collision_pairs.size()).to_be(6);
				expect(contains_collision(collision_pairs, 1, 2)).to_be(true);
				expect(contains_collision(collision_pairs, 2, 3)).to_be(true);
				expect(contains_collision(collision_pairs, 1, 3)).to_be(true);
				expect(contains_collision(collision_pairs, 11, 12)).to_be(true);
				expect(contains_collision(collision_pairs, 12, 13)).to_be(true);
				expect(contains_collision(collision_pairs, 11, 13)).to_be(true);
			});
		});
	});
}