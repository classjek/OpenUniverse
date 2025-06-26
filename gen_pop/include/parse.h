#pragma once

#include <string>
#include "kb_core.h"

namespace kb::parse {

// Top‑level API: parse one textual constraint such as
//   "Q(x,b) - 3 >= 0"
//   "R(y) + 2*Q(y,jack) = 0"
// into a kb::Constraint object.
// Throws std::runtime_error on syntax errors.
Constraint parseConstraint(const std::string &text);
}