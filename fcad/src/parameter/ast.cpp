#include "controllers/geometry.h"
#include "parameter/ast.h"
#include "parameter/eval.h"
#include "parsing.h"

namespace {
	std::optional<size_t> get_vert_idx_from_name(const std::wstring &name) {
		std::wstringstream wss{};
		wss << name;
		parsing::parser_state state(wss);
		std::optional<size_t> idx_opt = parse_explicit_vertex(state);

		if (! state.eof()) {
			return std::nullopt;
		}

		return idx_opt;
	}
}

std::wstring type_err_log::to_wstr() const {
	if (errors.empty()) {
		return L"";
	}

	return errors[errors.size() - 1];
}

expr::expr(expr_type _type) : type(_type) {}

void expr::typecheck(const eval_context&, type_err_log&) const {}

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

scalar_func_expr::scalar_func_expr(
	const std::wstring &_name,
	std::vector<std::unique_ptr<expr>> &&_args
) :
	scalar_expr(expr_type::ScalarFunc),
	name(_name),
	args(std::move(_args))
{}

phys::real scalar_func_expr::eval(const eval_context &ctx) const {
	std::vector<func_arg> arg_vals{};

	for (const auto &arg_expr : args) {
		const expr_class arg_t = arg_expr->get_expr_class();

		if (arg_t == expr_class::Scalar) {
			const scalar_expr * expr = static_cast<scalar_expr *>(arg_expr.get());

			arg_vals.push_back(expr->eval(ctx));
		} else {
			const vector_expr * expr = static_cast<vector_expr *>(arg_expr.get());

			arg_vals.push_back(expr->eval(ctx));
		}
	}

	const func_defn &defn = ctx.builtin_funcs->at(name);

	return std::get<phys::real>(defn.body(arg_vals));
}

bool scalar_func_expr::is_const() const {
	// Functions are pure
	for (const auto &arg : args) {
		if (! arg->is_const()) {
			return false;
		}
	}

	return true;
}

void scalar_func_expr::typecheck(const eval_context &ctx, type_err_log &log) const {
	using traits::to_string;
	using util::to_wstring;
	// TODO: traits::to_wstring

	if (! ctx.builtin_funcs->count(name)) {
		log.errors.push_back(L"\"" + name + L"\" is undefined");
		return;
	}

	const func_defn &defn = ctx.builtin_funcs->at(name);

	if (args.size() != defn.args_t.size()) {
		log.errors.push_back(
			L"Expected " + to_wstring(to_string(defn.args_t.size())) +
			L" arguments, received " + to_wstring(to_string(args.size()))
		);
		return;
	}

	for (size_t i = 0; i < args.size(); i++) {
		const expr_class exp_expr_t = defn.args_t.at(i);
		const expr_class actual_expr_t = args.at(i)->get_expr_class();

		if (actual_expr_t != exp_expr_t) {
			if (exp_expr_t == expr_class::Scalar) {
				log.errors.push_back(
					L"Expected scalar argument, received vector"
				);
			} else {
				log.errors.push_back(
					L"Expected vector argument, received scalar"
				);
			}
			return;
		}
	}

	if (defn.ret_t != expr_class::Scalar) {
		log.errors.push_back(
			L"Expected scalar expression"
		);
		return;
	}
}

vector_func_expr::vector_func_expr(
	const std::wstring &_name,
	std::vector<std::unique_ptr<expr>> &&_args
) :
	vector_expr(expr_type::VectorFunc),
	name(_name),
	args(std::move(_args)),
	vert_idx_opt(get_vert_idx_from_name(_name))
{}

phys::vec3 vector_func_expr::eval(const eval_context &ctx) const {
	if (vert_idx_opt) {
		const size_t vert_idx = *vert_idx_opt;
		std::optional<phys::vec3> pos_opt = ctx.geom->get_vertex_pos(vert_idx);

		if (! pos_opt) {
			throw undefined_vertex_error(vert_idx);
		}

		if (ctx.verts->count(vert_idx)) {
			const std::unique_ptr<vector_expr> &defn = ctx.verts->at(vert_idx);

			return defn->eval(ctx);
		}

		return *pos_opt;
	}

	std::vector<func_arg> arg_vals{};

	for (const auto &arg_expr : args) {
		const expr_class arg_t = arg_expr->get_expr_class();

		if (arg_t == expr_class::Scalar) {
			const scalar_expr * expr = static_cast<scalar_expr *>(arg_expr.get());

			arg_vals.push_back(expr->eval(ctx));
		} else {
			const vector_expr * expr = static_cast<vector_expr *>(arg_expr.get());

			arg_vals.push_back(expr->eval(ctx));
		}
	}

	const func_defn &defn = ctx.builtin_funcs->at(name);

	return std::get<phys::vec3>(defn.body(arg_vals));
}

bool vector_func_expr::is_const() const {
	if (vert_idx_opt) {
		// Vertex index functions are impure
		return false;
	}

	for (const auto &arg : args) {
		if (! arg->is_const()) {
			return false;
		}
	}

	return true;
}

void vector_func_expr::typecheck(const eval_context &ctx, type_err_log &log) const {
	using traits::to_string;
	using util::to_wstring;
	// TODO: traits::to_wstring

	if (vert_idx_opt) {
		if (! args.empty()) {
			log.errors.push_back(L"Unexpected arguments passed to zero-arg function");
		}

		return;
	}

	if (! ctx.builtin_funcs->count(name)) {
		log.errors.push_back(L"\"" + name + L"\" is undefined");
		return;
	}

	const func_defn &defn = ctx.builtin_funcs->at(name);

	if (args.size() != defn.args_t.size()) {
		log.errors.push_back(
			L"Expected " + to_wstring(to_string(defn.args_t.size())) +
			L" arguments, received " + to_wstring(to_string(args.size()))
		);
		return;
	}

	for (size_t i = 0; i < args.size(); i++) {
		const expr_class exp_expr_t = defn.args_t.at(i);
		const expr_class actual_expr_t = args.at(i)->get_expr_class();

		if (actual_expr_t != exp_expr_t) {
			if (exp_expr_t == expr_class::Scalar) {
				log.errors.push_back(
					L"Expected scalar argument, received vector"
				);
			} else {
				log.errors.push_back(
					L"Expected vector argument, received scalar"
				);
			}
			return;
		}
	}

	if (defn.ret_t != expr_class::Vector) {
		log.errors.push_back(
			L"Expected vector expression"
		);
		return;
	}
}

vector_ident_expr::vector_ident_expr(const std::wstring &_name) :
	vector_expr(expr_type::VectorIdent), name(_name)
{}

phys::vec3 vector_ident_expr::eval(const eval_context &ctx) const {
	if (! ctx.vectors->count(name)) {
		throw unknown_ident_error(name);
	}

	return ctx.vectors->at(name).defn->eval(ctx);
}

bool vector_ident_expr::is_const() const {
	return false;
}

vector_infix_expr::vector_infix_expr(
	expr_type _type,
	std::unique_ptr<vector_expr> &&_op1,
	std::unique_ptr<vector_expr> &&_op2
) :
	vector_expr(_type),
	op1(std::move(_op1)),
	op2(std::move(_op2))
{}

phys::vec3 vector_infix_expr::eval(const eval_context &ctx) const {
	return impl(op1->eval(ctx), op2->eval(ctx));
}

bool vector_infix_expr::is_const() const {
	return op1->is_const() && op2->is_const();
}

vector_add_expr::vector_add_expr(
	std::unique_ptr<vector_expr> &&_op1,
	std::unique_ptr<vector_expr> &&_op2
) :
	vector_infix_expr(
		expr_type::VectorAdd,
		std::move(_op1),
		std::move(_op2)
	)
{}

phys::vec3 vector_add_expr::impl(const phys::vec3 &r1, const phys::vec3 &r2) const {
	return r1 + r2;
}

vector_sub_expr::vector_sub_expr(
	std::unique_ptr<vector_expr> &&_op1,
	std::unique_ptr<vector_expr> &&_op2
) :
	vector_infix_expr(
		expr_type::VectorSub,
		std::move(_op1),
		std::move(_op2)
	)
{}

phys::vec3 vector_sub_expr::impl(const phys::vec3 &r1, const phys::vec3 &r2) const {
	return r1 - r2;
}

vector_cross_expr::vector_cross_expr(
	std::unique_ptr<vector_expr> &&_op1,
	std::unique_ptr<vector_expr> &&_op2
) :
	vector_infix_expr(
		expr_type::VectorCross,
		std::move(_op1),
		std::move(_op2)
	)
{}

phys::vec3 vector_cross_expr::impl(const phys::vec3 &r1, const phys::vec3 &r2) const {
	return phys::cross(r1, r2);
}

vector_dot_expr::vector_dot_expr(
	std::unique_ptr<vector_expr> &&_op1,
	std::unique_ptr<vector_expr> &&_op2
) :
	scalar_expr(expr_type::VectorDot),
	op1(std::move(_op1)),
	op2(std::move(_op2))
{}

phys::real vector_dot_expr::eval(const eval_context &ctx) const {
	return phys::dot(op1->eval(ctx), op2->eval(ctx));
}

bool vector_dot_expr::is_const() const {
	return op1->is_const() && op2->is_const();
}