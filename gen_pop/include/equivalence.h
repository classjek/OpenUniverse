#pragma once
#include <unordered_map>
#include <unordered_set>
#include "kb_core.h"      // for Sym, Constraint

namespace kb::eq {

std::unordered_map<std::string, MonoPtr> genMonomialMap(const std::vector<Constraint>& constraints);

std::vector<std::uint32_t> computeGenericClasses(const std::vector<Constraint>& constraints);

std::unordered_map<Sym, Sym> computeGroundNameClasses(
                        const std::vector<Constraint>& constraints,
                        const std::unordered_set<Sym>& groundNames);

std::unordered_map<Sym, std::string> relVarMap(const std::vector<Constraint>& constraints);

} 
