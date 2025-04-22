#pragma once
#include <map>
#include <vector>
#include "fcad_events.h"
#include "parameter/eval.h"
#include "traits.h"

class param_exists_error : public std::runtime_error {
	using std::runtime_error::runtime_error;
};

class param_does_not_exist_error : public std::runtime_error {
	using std::runtime_error::runtime_error;
};

class vertex_exists_error : public std::runtime_error {
	using std::runtime_error::runtime_error;
};

class renumber_collision_error : public std::runtime_error {
	using std::runtime_error::runtime_error;
};

class parameter_controller :
	traits::pinned<parameter_controller>,
	event_listener<fcad_start_event>
{
public:
	parameter_controller(fcad_event_bus &_events);

	void add_scalar_parameter(
		const std::wstring &name,
		std::unique_ptr<scalar_expr> &&defn
	);
	void set_scalar_parameter(
		const std::wstring &name,
		std::unique_ptr<scalar_expr> &&defn
	);
	void add_vector_parameter(
		const std::wstring &name,
		std::unique_ptr<vector_expr> &&defn
	);
	void set_vector_parameter(
		const std::wstring &name,
		std::unique_ptr<vector_expr> &&defn
	);
	type_err_log typecheck(const expr &e) const;
	bool create_vertex(std::unique_ptr<vector_expr> &&defn);
	bool bind_vertex(size_t vertex_idx, std::unique_ptr<vector_expr> &&defn);
	void remove_vertex(size_t vertex_idx);
	void renumber_vertex(size_t old_idx, size_t new_idx);
	void reset();

	int handle(fcad_start_event &event) override;

private:
	fcad_event_bus &events;
	geometry_controller * geom{};
	scalar_symbol_map scalars{};
	vector_symbol_map vectors{};
	std::map<size_t, std::unique_ptr<vector_expr>> vert_defns{};

	void scalar_move_verts(const std::wstring &name) const;
	const eval_context make_eval_context() const;
};
