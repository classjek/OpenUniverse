#pragma once
#include <unordered_map>
#include <unordered_set>
#include "kb_core.h"      // for Sym, Constraint

namespace kb::eq {

// what should this be? 
// Because eventually what? 
// We will go to make a grounding and how will we do that? 
// and what will this grounding be used for? 
// consider new problem formulation
std::unordered_map<std::string, MonoPtr> genMonomialMap(const std::vector<Constraint>& constraints);

std::vector<std::uint32_t> computeGenericClasses(const std::vector<Constraint>& constraints);

std::unordered_map<Sym, Sym> computeGroundNameClasses(
                        const std::vector<Constraint>& constraints,
                        const std::unordered_set<Sym>& groundNames,
                        const std::vector<std::uint32_t>& generic2class);

} 
