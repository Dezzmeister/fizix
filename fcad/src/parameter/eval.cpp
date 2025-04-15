#include "traits.h"
#include "parameter/eval.h"

unknown_ident_error::unknown_ident_error(const std::wstring &_ident) :
	std::runtime_error("Unknown identifier: " + traits::to_string(_ident)),
	ident(_ident)
{}