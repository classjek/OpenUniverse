#include "equivalence.h"
#include <iostream>

namespace kb::eq{
// Runs through collected constraints and generates a map from each monomial to a representative monomial.
// This is the initial map, so it simply takes each monomial to itself, but it will be updated while calculating equivalences
// And will eventually map each monomial to its equivalence class representative.
std::unordered_map<std::string, MonoPtr> genMonomialMap(const std::vector<Constraint>& constraints){
    std::unordered_map<std::string, MonoPtr> monomialMap;
    for (const auto& ct : constraints) {
        for (const auto& term : ct.poly.terms) { 
            const MonoPtr& monoPtr = term.first;
            // canonical representation lives once per monomial
            const std::string& sig = monoPtr->toString();
            // preserves the first pointer and ignores duplicates
            monomialMap.try_emplace(sig, monoPtr);
        }
    }
    return monomialMap;
}

std::vector<std::uint32_t> computeGenericClasses(const std::vector<Constraint>& constraints){
    return std::vector<std::uint32_t>();
}

std::unordered_map<Sym, Sym> computeGroundNameClasses(const std::vector<Constraint>& constraints, const std::unordered_set<Sym>& groundNames,const std::vector<std::uint32_t>& generic2class){
    return std::unordered_map<Sym, Sym>();
} 

}