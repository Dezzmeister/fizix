#pragma once
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>
#include "physics/math.h"

struct eval_context;

enum class expr_type {
	ScalarLiteral,
	ScalarIdent,
	ScalarAdd,
	ScalarSub,
	ScalarMul,
	ScalarDiv,
	ScalarFunc,
	VertexIdx,
	VectorLiteral,
	VectorIdent,
	VectorAdd,
	VectorSub,
	VectorCross,
	VectorDot,
	VectorFunc
};

enum class expr_class {
	Scalar,
	Vector
};

struct type_err_log {
	std::vector<std::wstring> errors{};

	std::wstring to_wstr() const;
};

class expr {
public:
	const expr_type type;

	virtual ~expr() = default;

	virtual bool is_const() const = 0;
	virtual expr_class get_expr_class() const = 0;
	virtual void typecheck(const eval_context &ctx, type_err_log &log) const;

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

class scalar_func_expr : public scalar_expr {
public:
	const std::wstring name;
	const std::vector<std::unique_ptr<expr>> args;

	scalar_func_expr(
		const std::wstring &_name,
		std::vector<std::unique_ptr<expr>> &&_args
	);

	phys::real eval(const eval_context &ctx) const override;
	bool is_const() const override;
	void typecheck(const eval_context &ctx, type_err_log &log) const override;
};

// TODO: Remove type information from grammar, unite scalar_func and vector_func
// Vector indices are actually impure vector functions named like v0, v1, etc.
class vector_func_expr : public vector_expr {
public:
	const std::wstring name;
	const std::vector<std::unique_ptr<expr>> args;
	const std::optional<size_t> vert_idx_opt;

	vector_func_expr(
		const std::wstring &_name,
		std::vector<std::unique_ptr<expr>> &&_args
	);

	phys::vec3 eval(const eval_context &ctx) const override;
	bool is_const() const override;
	void typecheck(const eval_context &ctx, type_err_log &log) const override;
};

class vector_ident_expr : public vector_expr {
public:
	const std::wstring name;

	vector_ident_expr(const std::wstring &_name);

	phys::vec3 eval(const eval_context &ctx) const override;
	bool is_const() const override;
};

// TODO: Generalized infix expr
class vector_infix_expr : public vector_expr {
public:
	const std::unique_ptr<vector_expr> op1;
	const std::unique_ptr<vector_expr> op2;

	vector_infix_expr(
		expr_type _type,
		std::unique_ptr<vector_expr> &&_op1,
		std::unique_ptr<vector_expr> &&_op2
	);

	virtual phys::vec3 eval(const eval_context &ctx) const override;
	virtual bool is_const() const override;

protected:
	virtual phys::vec3 impl(const phys::vec3 &r1, const phys::vec3 &r2) const = 0;
};

class vector_add_expr : public vector_infix_expr {
public:
	vector_add_expr(
		std::unique_ptr<vector_expr> &&_op1,
		std::unique_ptr<vector_expr> &&_op2
	);

protected:
	phys::vec3 impl(const phys::vec3 &r1, const phys::vec3 &r2) const;
};

class vector_sub_expr : public vector_infix_expr {
public:
	vector_sub_expr(
		std::unique_ptr<vector_expr> &&_op1,
		std::unique_ptr<vector_expr> &&_op2
	);

protected:
	phys::vec3 impl(const phys::vec3 &r1, const phys::vec3 &r2) const;
};

class vector_cross_expr : public vector_infix_expr {
public:
	vector_cross_expr(
		std::unique_ptr<vector_expr> &&_op1,
		std::unique_ptr<vector_expr> &&_op2
	);

protected:
	phys::vec3 impl(const phys::vec3 &r1, const phys::vec3 &r2) const;
};

class vector_dot_expr : public scalar_expr {
public:
	const std::unique_ptr<vector_expr> op1;
	const std::unique_ptr<vector_expr> op2;

	vector_dot_expr(
		std::unique_ptr<vector_expr> &&_op1,
		std::unique_ptr<vector_expr> &&_op2
	);

	phys::real eval(const eval_context &ctx) const override;
	bool is_const() const override;
};
