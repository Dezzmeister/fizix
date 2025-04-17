#include "controllers/geometry.h"
#include "parameter/ast.h"
#include "parameter/eval.h"

expr::expr(expr_type _type) : type(_type) {}

expr_class scalar_expr::get_expr_class() const {
	return expr_class::Scalar;
}

expr_class vector_expr::get_expr_class() const {
	return expr_class::Vector;
}

scalar_literal_expr::scalar_literal_expr(phys::real _val) :
	scalar_expr(expr_type::ScalarLiteral), val(_val)
{}

phys::real scalar_literal_expr::eval(const eval_context&) const {
	return val;
}

bool scalar_literal_expr::is_const() const {
	return true;
}

scalar_ident_expr::scalar_ident_expr(const std::wstring &_name) :
	scalar_expr(expr_type::ScalarIdent), name(_name)
{}

phys::real scalar_ident_expr::eval(const eval_context &ctx) const {
	if (! ctx.scalars->count(name)) {
		throw unknown_ident_error(name);
	}

	return ctx.scalars->at(name).defn->eval(ctx);
}

bool scalar_ident_expr::is_const() const {
	return false;
}

scalar_infix_expr::scalar_infix_expr(
	expr_type _type,
	std::unique_ptr<scalar_expr> &&_op1,
	std::unique_ptr<scalar_expr> &&_op2
) :
	scalar_expr(_type),
	op1(std::move(_op1)),
	op2(std::move(_op2))
{}

phys::real scalar_infix_expr::eval(const eval_context &ctx) const {
	return impl(op1->eval(ctx), op2->eval(ctx));
}

bool scalar_infix_expr::is_const() const {
	return op1->is_const() && op2->is_const();
}

scalar_add_expr::scalar_add_expr(
	std::unique_ptr<scalar_expr> &&_op1,
	std::unique_ptr<scalar_expr> &&_op2
) :
	scalar_infix_expr(
		expr_type::ScalarAdd,
		std::move(_op1),
		std::move(_op2)
	)
{}

phys::real scalar_add_expr::impl(phys::real r1, phys::real r2) const {
	return r1 + r2;
}

scalar_sub_expr::scalar_sub_expr(
	std::unique_ptr<scalar_expr> &&_op1,
	std::unique_ptr<scalar_expr> &&_op2
) :
	scalar_infix_expr(
		expr_type::ScalarSub,
		std::move(_op1),
		std::move(_op2)
	)
{}

phys::real scalar_sub_expr::impl(phys::real r1, phys::real r2) const {
	return r1 - r2;
}

scalar_mul_expr::scalar_mul_expr(
	std::unique_ptr<scalar_expr> &&_op1,
	std::unique_ptr<scalar_expr> &&_op2
) :
	scalar_infix_expr(
		expr_type::ScalarMul,
		std::move(_op1),
		std::move(_op2)
	)
{}

phys::real scalar_mul_expr::impl(phys::real r1, phys::real r2) const {
	return r1 * r2;
}

scalar_div_expr::scalar_div_expr(
	std::unique_ptr<scalar_expr> &&_op1,
	std::unique_ptr<scalar_expr> &&_op2
) :
	scalar_infix_expr(
		expr_type::ScalarDiv,
		std::move(_op1),
		std::move(_op2)
	)
{}

phys::real scalar_div_expr::impl(phys::real r1, phys::real r2) const {
	return r1 / r2;
}

vertex_idx_expr::vertex_idx_expr(size_t _vertex_idx) :
	vector_expr(expr_type::VertexIdx),
	vertex_idx(_vertex_idx)
{}

phys::vec3 vertex_idx_expr::eval(const eval_context &ctx) const {
	if (ctx.verts->count(vertex_idx)) {
		const std::unique_ptr<vector_expr> &defn = ctx.verts->at(vertex_idx);

		return defn->eval(ctx);
	}

	return *(ctx.geom->get_vertex_pos(vertex_idx));
}

bool vertex_idx_expr::is_const() const {
	return false;
}

vector_literal_expr::vector_literal_expr(
	std::unique_ptr<scalar_expr> &&_x,
	std::unique_ptr<scalar_expr> &&_y,
	std::unique_ptr<scalar_expr> &&_z
) :
	vector_expr(expr_type::VectorLiteral),
	x(std::move(_x)),
	y(std::move(_y)),
	z(std::move(_z))
{}

phys::vec3 vector_literal_expr::eval(const eval_context &ctx) const {
	return phys::vec3(
		x->eval(ctx),
		y->eval(ctx),
		z->eval(ctx)
	);
}

bool vector_literal_expr::is_const() const {
	return x->is_const() && y->is_const() && z->is_const();
}