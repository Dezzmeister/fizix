#pragma once
#include <memory>
#include <set>
#include <string>
#include "physics/math.h"

struct eval_context;

enum expr_type {
	ScalarLiteral,
	ScalarIdent,
	VectorLiteral
};

class expr {
public:
	const expr_type type;

	virtual ~expr() = default;

	virtual bool is_const() const = 0;

protected:
	expr(expr_type _type);
};

class scalar_expr : public expr {
public:
	virtual ~scalar_expr() = default;

	virtual phys::real eval(const eval_context &ctx) const = 0;
	virtual void get_idents(std::set<std::wstring> &idents) const = 0;
	virtual bool depends_on_ident(const std::wstring &ident) const = 0;
	virtual bool is_const() const = 0;

protected:
	using expr::expr;
};

class vector_expr : public expr {
public:
	virtual ~vector_expr() = default;

	virtual phys::vec3 eval(const eval_context &ctx) const = 0;
	virtual bool depends_on_ident(const std::wstring &ident) const = 0;
	virtual bool is_const() const = 0;

protected:
	using expr::expr;
};

class scalar_literal_expr : public scalar_expr {
public:
	const phys::real val;

	scalar_literal_expr(phys::real _val);

	phys::real eval(const eval_context &ctx) const override;
	void get_idents(std::set<std::wstring> &idents) const override;
	bool depends_on_ident(const std::wstring &ident) const override;
	bool is_const() const override;
};

class scalar_ident_expr : public scalar_expr {
public:
	const std::wstring name;

	scalar_ident_expr(const std::wstring &_name);

	phys::real eval(const eval_context &ctx) const override;
	void get_idents(std::set<std::wstring> &_idents) const override;
	bool depends_on_ident(const std::wstring &ident) const override;
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
	bool depends_on_ident(const std::wstring &ident) const override;
	bool is_const() const override;
};