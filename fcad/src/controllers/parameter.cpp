#include "controllers/geometry.h"
#include "controllers/parameter.h"
#include "parameter/parser.h"

namespace {
	func_defn_map builtin_funcs{
		{L"x", func_defn(
			L"x",
			{ expr_class::Vector },
			expr_class::Scalar,
			[](const args_obj &args) {
				phys::vec3 v = std::get<phys::vec3>(args[0]);

				return v.x;
			}
		)},
		{L"y", func_defn(
			L"y",
			{ expr_class::Vector },
			expr_class::Scalar,
			[](const args_obj &args) {
				phys::vec3 v = std::get<phys::vec3>(args[0]);

				return v.y;
			}
		)},
		{L"z", func_defn(
			L"z",
			{ expr_class::Vector },
			expr_class::Scalar,
			[](const args_obj &args) {
				phys::vec3 v = std::get<phys::vec3>(args[0]);

				return v.z;
			}
		)}
	};
}

parameter_controller::parameter_controller(
	fcad_event_bus &_events
) :
	event_listener<fcad_start_event>(&_events),
	events(_events)
{
	event_listener<fcad_start_event>::subscribe();
}

void parameter_controller::add_scalar_parameter(
	const std::wstring &name,
	std::unique_ptr<scalar_expr> &&defn
) {
	if (scalars.count(name)) {
		throw param_exists_error("Parameter already exists!");
	}

	scalars[name] = scalar_parameter{
		.defn = std::move(defn)
	};
}

void parameter_controller::set_scalar_parameter(
	const std::wstring &name,
	std::unique_ptr<scalar_expr> &&defn
) {
	if (! scalars.count(name)) {
		throw param_does_not_exist_error("Parameter does not exist!");
	}

	scalars[name] = scalar_parameter{
		.defn = std::move(defn)
	};

	scalar_move_verts(name);
}

void parameter_controller::add_vector_parameter(
	const std::wstring &name,
	std::unique_ptr<vector_expr> &&defn
) {
	if (vectors.count(name)) {
		throw param_exists_error("Parameter already exists!");
	}

	vectors[name] = vector_parameter{
		.defn = std::move(defn)
	};
}

void parameter_controller::set_vector_parameter(
	const std::wstring &name,
	std::unique_ptr<vector_expr> &&defn
) {
	if (! vectors.count(name)) {
		throw param_does_not_exist_error("Parameter does not exist!");
	}

	vectors[name] = vector_parameter{
		.defn = std::move(defn)
	};

	scalar_move_verts(name);
}

type_err_log parameter_controller::typecheck(const expr &e) const {
	type_err_log out{};

	e.typecheck(make_eval_context(), out);

	return out;
}

bool parameter_controller::create_vertex(std::unique_ptr<vector_expr> &&defn) {
	using traits::to_string;

	phys::vec3 pos_now = defn->eval(make_eval_context());

	if (defn->is_const()) {
		geom->create_vertex(pos_now, true);
		return true;
	}

	int64_t vertex_idx = geom->create_vertex(pos_now, true);

	if (vertex_idx == -1) {
		return false;
	}

	if (vert_defns.count(vertex_idx)) {
		throw vertex_exists_error("Vertex " + to_string(vertex_idx) + " already exists!");
	}

	vert_defns[vertex_idx] = std::move(defn);

	return true;
}

bool parameter_controller::bind_vertex(size_t vertex_idx, std::unique_ptr<vector_expr> &&defn) {
	using traits::to_string;

	phys::vec3 new_pos = defn->eval(make_eval_context());

	vert_defns[vertex_idx] = std::move(defn);

	return geom->move_vertex(vertex_idx, new_pos, true);
}

void parameter_controller::remove_vertex(size_t vertex_idx) {
	if (! vert_defns.count(vertex_idx)) {
		return;
	}

	vert_defns.erase(vertex_idx);
}

void parameter_controller::renumber_vertex(size_t old_idx, size_t new_idx) {
	if (! vert_defns.count(old_idx)) {
		return;
	}

	if (vert_defns.count(new_idx)) {
		throw renumber_collision_error("New vertex is already defined!");
	}

	vert_defns[new_idx] = std::move(vert_defns.at(old_idx));
	vert_defns.erase(old_idx);
}

void parameter_controller::reset() {
	scalars.clear();
	vectors.clear();
	vert_defns.clear();
}

int parameter_controller::handle(fcad_start_event &event) {
	geom = &event.gc;

	return 0;
}

void parameter_controller::scalar_move_verts(const std::wstring&) const {
	using traits::to_string;

	const eval_context ctx = make_eval_context();

	for (auto const &pair : vert_defns) {
		const std::optional<phys::vec3> curr_pos_opt = geom->get_vertex_pos(pair.first);

		if (! curr_pos_opt) {
			logger::error("Vertex " + to_string(pair.first) + " missing from parameter controller");
			continue;
		}

		const phys::vec3 curr_pos = *curr_pos_opt;
		const phys::vec3 new_pos = pair.second->eval(ctx);

		if (curr_pos == new_pos) {
			continue;
		}

		geom->move_vertex(pair.first, new_pos, true);
	}
}

const eval_context parameter_controller::make_eval_context() const {
	return eval_context(
		&scalars,
		&vectors,
		&vert_defns,
		geom,
		&builtin_funcs
	);
}
