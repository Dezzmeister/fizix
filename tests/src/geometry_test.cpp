#include <physics/math_util.h>
#include <physics/collision/vclip.h>
#include "test.h"

using namespace test;
using namespace phys;
using namespace phys::vclip;
using namespace phys::literals;

void setup_geometry_tests() {
	describe("Geometry", []() {
		describe("support routines", []() {
			describe("line/line intersection", []() {
				it("reports intersection for lines (XZ) (1)", []() {
					std::optional<vec2> result = line_line_intersection(
						vec3(0.0_r),
						vec3(1.0_r, 0.0_r, 0.0_r),
						vec3(0.5_r, 0.0_r, -0.5_r),
						vec3(0.0_r, 0.0_r, 1.0_r)
					);

					expect(result).to_be(vec2(0.5_r, 0.5_r));
				});

				it("reports intersection for lines (XZ) (2)", []() {
					std::optional<vec2> result = line_line_intersection(
						vec3(-1.0_r, 0.0_r, 0.0_r),
						vec3(2.0_r, 0.0_r, 2.0_r),
						vec3(1.0_r, 0.0_r, 0.0_r),
						vec3(-2.0_r, 0.0_r, 2.0_r)
					);

					expect(result).to_be(vec2(0.5_r, 0.5_r));
				});

				it("reports intersection for lines (XY)", []() {
					std::optional<vec2> result = line_line_intersection(
						vec3(-1.0_r, 0.0_r, 0.0_r),
						vec3(1.0_r, 1.0_r, 0.0_r),
						vec3(1.0_r, 0.0_r, 0.0_r),
						vec3(-1.0_r, 1.0_r, 0.0_r)
					);

					expect(result).to_be(vec2(1.0_r, 1.0_r));
				});

				it("reports intersection for lines (YZ)", []() {
					std::optional<vec2> result = line_line_intersection(
						vec3(0.0_r, 0.0_r, -1.0_r),
						vec3(0.0_r, 2.0_r, 2.0_r),
						vec3(0.0_r, 0.0_r, 1.0_r),
						vec3(0.0_r, 2.0_r, -2.0_r)
					);

					expect(result).to_be(vec2(0.5_r, 0.5_r));
				});

				it("reports intersection for lines (YZ) (2)", []() {
					std::optional<vec2> result = line_line_intersection(
						vec3(0.0_r, 1.0_r, -1.0_r),
						vec3(0.0_r, 1.0_r, 1.0_r),
						vec3(0.0_r, 1.0_r, 0.0_r),
						vec3(0.0_r, 1.0_r, 0.0_r)
					);

					expect(result).to_be(vec2(1.0_r, 1.0_r));
				});

				it("reports intersection for lines (not aligned to axes)", []() {
					std::optional<vec2> result = line_line_intersection(
						vec3(-1.0_r, -1.0_r, -1.0_r),
						vec3(2.0_r, 2.0_r, 2.0_r),
						vec3(-1.0_r, -1.0_r, 1.0_r),
						vec3(2.0_r, 2.0_r, -2.0_r)
					);

					expect(result).to_be(vec2(0.5_r, 0.5_r));
				});

				it("reports non-intersection for lines that meet each other at t_i > 1 (XZ)", []() {
					std::optional<vec2> result = line_line_intersection(
						vec3(-1.0_r, 0.0_r, 0.0_r),
						vec3(0.5_r, 0.0_r, 0.5_r),
						vec3(1.0_r, 0.0_r, 0.0_r),
						vec3(-0.5_r, 0.0_r, 0.5_r)
					);

					expect(result).to_be(vec2(2.0_r, 2.0_r));
				});

				it("reports non-intersection for lines that meet each other at t_i > 1 (XY)", []() {
					std::optional<vec2> result = line_line_intersection(
						vec3(-1.0_r, 0.0_r, 0.0_r),
						vec3(0.5_r, 0.5_r, 0.0_r),
						vec3(1.0_r, 0.0_r, 0.0_r),
						vec3(-0.5_r, 0.5_r, 0.0_r)
					);

					expect(result).to_be(vec2(2.0_r, 2.0_r));
				});

				it("reports non-intersection for lines that meet each other at t_i > 1 (YZ)", []() {
					std::optional<vec2> result = line_line_intersection(
						vec3(0.0_r, 0.0_r, -1.0_r),
						vec3(0.0_r, 0.5_r, 0.5_r),
						vec3(0.0_r, 0.0_r, 1.0_r),
						vec3(0.0_r, 0.5_r, -0.5_r)
					);

					expect(result).to_be(vec2(2.0_r, 2.0_r));
				});

				it("reports non-intersection for lines that meet each other at t_i > 1 "
					"not aligned to axes", []() {

					std::optional<vec2> result = line_line_intersection(
						vec3(-1.0_r, -1.0_r, -1.0_r),
						vec3(0.5_r, 0.5_r, 0.5_r),
						vec3(-1.0_r, -1.0_r, 1.0_r),
						vec3(0.5_r, 0.5_r, -0.5_r)
					);

					expect(result).to_be(vec2(2.0_r, 2.0_r));
				});

				it("reports non-intersection for lines that meet each other at 0 < t1 < 1 "
					"and t2 > 1", []() {
					std::optional<vec2> result = line_line_intersection(
						vec3(1.0_r, 0.0_r, -1.0_r),
						vec3(0.0_r, 0.0_r, 2.0_r),
						vec3(-4.0_r, 0.0_r, 0.0_r),
						vec3(1.0_r, 0.0_r, 0.0_r)
					);

					expect(result).to_be(vec2(0.5_r, 5.0_r));
				});

				it("reports no solution for parallel lines", []() {
					std::optional<vec2> result = line_line_intersection(
						vec3(1.0_r, 0.0_r, -1.0_r),
						vec3(1.0_r, 1.0_r, 1.0_r),
						vec3(0.5_r, 1.0_r, 1.0_r),
						vec3(1.0_r, 1.0_r, 1.0_r)
					);

					expect(result).to_be_empty();
				});

				it("reports no unique solution for colinear lines", []() {
					std::optional<vec2> result = line_line_intersection(
						vec3(0.0_r, -1.0_r, 0.0_r),
						vec3(0.0_r, 1.0_r, 0.0_r),
						vec3(0.0_r, 1.0_r, 0.0_r),
						vec3(0.0_r, 1.0_r, 0.0_r)
					);

					expect(result).to_be_empty();
				});
			});
		});

		describe("polyhedron", []() {
			it("adds a face and new edges to a polyhedron", []() {
				polyhedron p{
					.vertices = {
						vertex(vec3(0.0_r), 0),
						vertex(vec3(0.0_r, 1.0_r, 0.0_r), 1),
						vertex(vec3(0.0_r, 0.0_r, -1.0_r), 2)
					}
				};

				p.add_face_and_new_edges(face({ 0, 1, 2 }));

				expect(p.faces).to_have_size(1).annd()
					.to_have_item(face({ 0, 1, 2 }));
				expect(p.edges).to_have_size(3).annd()
					.to_have_item(edge(0, 1)).annd()
					.to_have_item(edge(1, 2)).annd()
					.to_have_item(edge(2, 0));
			});
		});
		describe("face", []() {
			it("computes the normal for a non-convex face (1)", []() {
				polyhedron p{
					.vertices = {
						vertex(vec3(0.0_r, 0.0_r, 0.2_r), 0),
						vertex(vec3(0.0_r, 5.0_r, 5.0_r), 1),
						vertex(vec3(0.0_r, 0.2_r, 0.0_r), 2),
						vertex(vec3(0.0_r, 5.0_r, -5.0_r), 3),
						vertex(vec3(0.0_r, 0.0_r, -0.2_r), 4),
						vertex(vec3(0.0_r, -5.0_r, -5.0_r), 5),
						vertex(vec3(0.0_r, -0.2_r, 0.0_r), 6),
						vertex(vec3(0.0_r, -5.0_r, 5.0_r), 7),
					}
				};

				p.add_face_and_new_edges(face({ 0, 1, 2, 3, 4, 5, 6, 7 }, convexity::Nonconvex));

				expect(p.edges).to_have_size(8);
				expect(face({ 0, 1, 2, 3, 4, 5, 6, 7 }, convexity::Nonconvex).normal(p))
					.to_be(vec3(-1.0_r, 0.0_r, 0.0_r));
				expect(face({ 1, 2, 3, 4, 5, 6, 7, 0 }, convexity::Nonconvex).normal(p))
					.to_be(vec3(-1.0_r, 0.0_r, 0.0_r));
				expect(face({ 2, 3, 4, 5, 6, 7, 0, 1 }, convexity::Nonconvex).normal(p))
					.to_be(vec3(-1.0_r, 0.0_r, 0.0_r));
				expect(face({ 3, 4, 5, 6, 7, 0, 1, 2 }, convexity::Nonconvex).normal(p))
					.to_be(vec3(-1.0_r, 0.0_r, 0.0_r));
				expect(face({ 4, 5, 6, 7, 0, 1, 2, 3 }, convexity::Nonconvex).normal(p))
					.to_be(vec3(-1.0_r, 0.0_r, 0.0_r));
				expect(face({ 5, 6, 7, 0, 1, 2, 3, 4 }, convexity::Nonconvex).normal(p))
					.to_be(vec3(-1.0_r, 0.0_r, 0.0_r));
				expect(face({ 6, 7, 0, 1, 2, 3, 4, 5 }, convexity::Nonconvex).normal(p))
					.to_be(vec3(-1.0_r, 0.0_r, 0.0_r));
				expect(face({ 7, 0, 1, 2, 3, 4, 5, 6 }, convexity::Nonconvex).normal(p))
					.to_be(vec3(-1.0_r, 0.0_r, 0.0_r));
			});

			it("computes the normal for a non-convex face (2)", []() {
				polyhedron p{
					.vertices = {
						vertex(vec3(0.0_r, 0.0_r, 0.0_r), 0),
						vertex(vec3(5.0_r, 0.0_r, 1.0_r), 1),
						vertex(vec3(0.2_r, 0.0_r, 0.0_r), 2),
						vertex(vec3(5.0_r, 0.0_r, -1.0_r), 3)
					}
				};

				p.add_face_and_new_edges(face({ 0, 1, 2, 3 }, convexity::Nonconvex));

				expect(p.edges).to_have_size(4);
				expect(face({ 0, 1, 2, 3 }, convexity::Nonconvex).normal(p))
					.to_be(vec3(0.0_r, 1.0_r, 0.0_r));
				expect(face({ 1, 2, 3, 0 }, convexity::Nonconvex).normal(p))
					.to_be(vec3(0.0_r, 1.0_r, 0.0_r));
				expect(face({ 2, 3, 0, 1 }, convexity::Nonconvex).normal(p))
					.to_be(vec3(0.0_r, 1.0_r, 0.0_r));
				expect(face({ 3, 0, 1, 2 }, convexity::Nonconvex).normal(p))
					.to_be(vec3(0.0_r, 1.0_r, 0.0_r));
			});

			it("identifies non-convex vertices (1)", []() {
				polyhedron p{
					.vertices = {
						vertex(vec3(0.0_r, 0.0_r, 0.2_r), 0),
						vertex(vec3(0.0_r, 5.0_r, 5.0_r), 1),
						vertex(vec3(0.0_r, 0.2_r, 0.0_r), 2),
						vertex(vec3(0.0_r, 5.0_r, -5.0_r), 3),
						vertex(vec3(0.0_r, 0.0_r, -0.2_r), 4),
						vertex(vec3(0.0_r, -5.0_r, -5.0_r), 5),
						vertex(vec3(0.0_r, -0.2_r, 0.0_r), 6),
						vertex(vec3(0.0_r, -5.0_r, 5.0_r), 7),
					}
				};

				face f({ 0, 1, 2, 3, 4, 5, 6, 7 }, convexity::Nonconvex);

				p.add_face_and_new_edges(f);

				expect(f.is_vertex_convex(p, 0)).to_be(false);
				expect(f.is_vertex_convex(p, 1)).to_be(true);
				expect(f.is_vertex_convex(p, 2)).to_be(false);
				expect(f.is_vertex_convex(p, 3)).to_be(true);
				expect(f.is_vertex_convex(p, 4)).to_be(false);
				expect(f.is_vertex_convex(p, 5)).to_be(true);
				expect(f.is_vertex_convex(p, 6)).to_be(false);
				expect(f.is_vertex_convex(p, 7)).to_be(true);
				expect(f.is_convex(p)).to_be(false);
				expect(face({ 0, 1, 2, 3, 4, 5, 6, 7 }, convexity::Unspecified).is_convex(p)).to_be(false);
			});

			it("identifies non-convex vertices (2)", []() {
				polyhedron p{
					.vertices = {
						vertex(vec3(0.0_r, 0.0_r, 0.0_r), 0),
						vertex(vec3(5.0_r, 0.0_r, 1.0_r), 1),
						vertex(vec3(0.2_r, 0.0_r, 0.0_r), 2),
						vertex(vec3(5.0_r, 0.0_r, -1.0_r), 3)
					}
				};

				face f({ 0, 1, 2, 3 }, convexity::Nonconvex);

				expect(f.is_vertex_convex(p, 0)).to_be(true);
				expect(f.is_vertex_convex(p, 1)).to_be(true);
				expect(f.is_vertex_convex(p, 2)).to_be(false);
				expect(f.is_vertex_convex(p, 3)).to_be(true);
				expect(f.is_convex(p)).to_be(false);
				expect(face({ 0, 1, 2, 3 }, convexity::Unspecified).is_convex(p)).to_be(false);
			});

			it("identifies non-convex vertices (3)", []() {
				polyhedron p{
					.vertices = {
						vertex(vec3(0.0_r, 0.0_r, 1.0_r), 0),
						vertex(vec3(1.0_r, 0.0_r, 0.0_r), 1),
						vertex(vec3(0.0_r, 0.0_r, -1.0_r), 2)
					}
				};

				face f({ 0, 1, 2 }, convexity::Unspecified);

				expect(f.is_vertex_convex(p, 0)).to_be(true);
				expect(f.is_vertex_convex(p, 1)).to_be(true);
				expect(f.is_vertex_convex(p, 2)).to_be(true);
				expect(f.is_convex(p)).to_be(true);
			});
		});
	});
}