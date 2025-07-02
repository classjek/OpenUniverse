#include "equivalence.h"
#include <iostream>
#include <map>

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

// Given a vector like those in appearanceMap, generate a hash 
uint64_t hashVec(const std::vector<uint64_t>& v) {
    uint64_t h = v.size();
    for (uint64_t x : v)
        h = (h << 1) ^ (x + 0x9e3779b97f4a7c15ULL + (h >> 3));
    return h;
}

// Given a vector representing distinctness constraints (i.e. Constraint.neq), form a hash 
size_t hashDistinct(const std::vector<std::pair<Sym,Sym>>& neq) {
    std::size_t h = 0xcbf29ce484222325ULL;            // FNV-1a offset
    constexpr std::size_t prime = 0x100000001b3ULL;   // FNV-1a prime
    for (auto [u,v] : neq) {
        if (v < u) std::swap(u,v);        // make (u,v) canonical, so (y,x)≡(x,y)
        std::size_t hu = std::hash<Sym>{}(u);
        std::size_t hv = std::hash<Sym>{}(v);
        h ^= hu;  h *= prime;
        h ^= hv;  h *= prime;
    }
    return h;
}

// Lowkey maybe just have this return a list of shrunk constraints 
std::unordered_map<Sym, Sym> computeGroundNameClasses(const std::vector<Constraint>& constraints, const std::unordered_set<std::string>& groundNames){
    // for each groundName, have a vector of hashes
    std::map<Sym, std::vector<uint64_t>> appearanceMap; 
    // Look through all constraints
    for (const auto& constraint : constraints){
        for (const auto& term : constraint.poly.terms) {
            const MonoPtr& monoPtr = term.first;
            if (monoPtr->isZero()) continue; // skip zero monomials
            for (const auto& item : monoPtr->items) { // look through each atom in monomial
                const std::vector<Sym>& atomArgs = item.first->args;
                for (const Sym& arg : atomArgs) {
                    if (groundNames.find(arg) != groundNames.end()) { // if arg is a ground name
                        // add that arg to the map if not already there
                        auto& vec = appearanceMap.try_emplace(arg).first->second;
                        auto poly_hash = std::hash<std::string>{}(constraint.poly.replaceString(arg)); // generate hash from replacedString
                        auto neq_hash = hashDistinct(constraint.neq); // generate hash from distinctness guard
                        std::size_t combined_hash = poly_hash ^ (neq_hash + 0x9e3779b97f4a7c15ULL + (poly_hash<<6) + (poly_hash>>2)); // combine both hashes
                        vec.push_back(combined_hash); // add the hash to groundName's vector
                        // vec.push_back(poly_hash); // add the hash of the replaced string to the vector
                    }
                }
                
            }
        }
    }

    // Now that all constraints have been searched through 
    std::unordered_map<uint64_t, std::vector<Sym>> classes;
    for (auto& [name, vec] : appearanceMap) { // search through appearanceMap
        // sort and remove duplicates
        std::sort(vec.begin(), vec.end());
        vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
        uint64_t h = hashVec(vec); // hash resulting vector

        auto it = classes.find(h);
        if (it == classes.end()) { // if hash is not present in map, add it and vector with name
            classes.emplace(h, std::vector<Sym>{name});
        } else { // hash already exists in map
            it->second.push_back(name); // add name to vector of names
        }
    }

    for (auto& [_, vec] : classes){
        std::cout << "Class: "; 
        for (std::size_t i = 0; i < vec.size(); i++){
            std::cout << vec[i] << ", ";
        }
        std::cout << " " << std::endl;
    }

    return std::unordered_map<Sym, Sym>();
} 

}