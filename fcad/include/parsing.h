#pragma once
#include <optional>
#include <data_formats/parsing.h>
#include <physics/collision/vclip.h>

std::optional<phys::real> parse_real(parsing::parser_state &state);
