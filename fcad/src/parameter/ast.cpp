#include "parameter/ast.h"
#include "parameter/eval.h"

expr::expr(expr_type _type) : type(_type) {}

scalar_literal_expr::scalar_literal_expr(phys::real _val) :
	scalar_expr(expr_type::ScalarLiteral), val(_val)
{}

phys::real scalar_literal_expr::eval(const eval_context&) const {
	return val;
}

void scalar_literal_expr::get_idents(std::set<std::wstring>&) const {}

bool scalar_literal_expr::depends_on_ident(const std::wstring&) const {
	return false;
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

void scalar_ident_expr::get_idents(std::set<std::wstring> &_idents) const {
	_idents.insert(name);
}

bool scalar_ident_expr::depends_on_ident(const std::wstring &ident) const {
	return ident == name;
}

bool scalar_ident_expr::is_const() const {
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

bool vector_literal_expr::depends_on_ident(const std::wstring &ident) const {
	return x->depends_on_ident(ident) || y->depends_on_ident(ident) || z->depends_on_ident(ident);
}

bool vector_literal_expr::is_const() const {
	return x->is_const() && y->is_const() && z->is_const();
}