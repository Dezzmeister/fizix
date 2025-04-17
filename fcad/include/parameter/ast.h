#pragma once
#include <memory>
#include <set>
#include <string>
#include "physics/math.h"

struct eval_context;

enum class expr_type {
	ScalarLiteral,
	ScalarIdent,
	ScalarAdd,
	ScalarSub,
	ScalarMul,
	ScalarDiv,
	VertexIdx,
	VectorLiteral
};

enum class expr_class {
	Scalar,
	Vector
};

class expr {
public:
	const expr_type type;

	virtual ~expr() = default;

	virtual bool is_const() const = 0;
	virtual expr_class get_expr_class() const = 0;

protected:
	expr(expr_type _type);
};

class scalar_expr : public expr {
public:
	virtual ~scalar_expr() = default;

	virtual phys::real eval(const eval_context &ctx) const = 0;
	virtual bool is_const() const = 0;
	virtual expr_class get_expr_class() const override;

protected:
	using expr::expr;
};

class vector_expr : public expr {
public:
	virtual ~vector_expr() = default;

	virtual phys::vec3 eval(const eval_context &ctx) const = 0;
	virtual bool is_const() const = 0;
	virtual expr_class get_expr_class() const override;

protected:
	using expr::expr;
};

class scalar_literal_expr : public scalar_expr {
public:
	const phys::real val;

	scalar_literal_expr(phys::real _val);

	phys::real eval(const eval_context &ctx) const override;
	bool is_const() const override;
};

class scalar_ident_expr : public scalar_expr {
public:
	const std::wstring name;

	scalar_ident_expr(const std::wstring &_name);

	phys::real eval(const eval_context &ctx) const override;
	bool is_const() const override;
};

class scalar_infix_expr : public scalar_expr {
public:
	const std::unique_ptr<scalar_expr> op1;
	const std::unique_ptr<scalar_expr> op2;

	scalar_infix_expr(
		expr_type _type,
		std::unique_ptr<scalar_expr> &&_op1,
		std::unique_ptr<scalar_expr> &&_op2
	);

	virtual phys::real eval(const eval_context &ctx) const override;
	virtual bool is_const() const override;

protected:
	virtual phys::real impl(phys::real r1, phys::real r2) const = 0;
};

class scalar_add_expr : public scalar_infix_expr {
public:
	scalar_add_expr(
		std::unique_ptr<scalar_expr> &&_op1,
		std::unique_ptr<scalar_expr> &&_op2
	);

protected:
	phys::real impl(phys::real r1, phys::real r2) const override;
};

class scalar_sub_expr : public scalar_infix_expr {
public:
	scalar_sub_expr(
		std::unique_ptr<scalar_expr> &&_op1,
		std::unique_ptr<scalar_expr> &&_op2
	);

protected:
	phys::real impl(phys::real r1, phys::real r2) const override;
};

class scalar_mul_expr : public scalar_infix_expr {
public:
	scalar_mul_expr(
		std::unique_ptr<scalar_expr> &&_op1,
		std::unique_ptr<scalar_expr> &&_op2
	);

protected:
	phys::real impl(phys::real r1, phys::real r2) const override;
};

class scalar_div_expr : public scalar_infix_expr {
public:
	scalar_div_expr(
		std::unique_ptr<scalar_expr> &&_op1,
		std::unique_ptr<scalar_expr> &&_op2
	);

protected:
	phys::real impl(phys::real r1, phys::real r2) const override;
};

class vertex_idx_expr : public vector_expr {
public:
	const size_t vertex_idx;

	vertex_idx_expr(size_t _vertex_idx);

	phys::vec3 eval(const eval_context &ctx) const override;
	bool is_const() const override;
};

class vector_literal_expr : public vector_expr {
public:
	const std::unique_ptr<scalar_expr> x;
	const std::unique_ptr<scalar_expr> y;
	const std::unique_ptr<scalar_expr> z;

	vector_literal_expr(
		std::unique_ptr<scalar_expr> &&_x,
		std::unique_ptr<scalar_expr> &&_y,
		std::unique_ptr<scalar_expr> &&_z
	);

	phys::vec3 eval(const eval_context &ctx) const override;
	bool is_const() const override;
};