#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>
#include <mutex>
#include <utility>
#include <algorithm>
#include <memory>

namespace kb{
//using SymID = std::uint32_t; //Do later for map-optimization
using Sym = std::string; 

struct Atom {
    Sym rel;                      
    std::vector<Sym> args;        

    std::string toString() const; 
    // define operations for comparing Atoms
    bool operator<(const Atom &o) const noexcept;
    bool operator==(const Atom &o) const noexcept;
};


using AtomPtr = std::shared_ptr<Atom>;

using Exponent = std::uint16_t;
using MonoItem = std::pair<std::shared_ptr<Atom>, Exponent>;


struct Monomial {
    std::vector<MonoItem> items;   // kept lexicographically sorted on Atom
    void canonicalise();           // sort + merge same atom (implementation later)

    std::string toString() const;
    static std::shared_ptr<Monomial> fromAtom(const AtomPtr& a);
    static std::shared_ptr<Monomial> multiply(const std::shared_ptr<Monomial>& A,
                                              const std::shared_ptr<Monomial>& B);
    std::vector<AtomPtr> expandedAtoms() const;        // debug helper

    bool operator<(const Monomial& o) const noexcept;  // lexicographic on items
    bool operator==(const Monomial& o) const noexcept;
};

using MonoPtr = std::shared_ptr<Monomial>;


using Coeff = std::int64_t;
using Term  = std::pair<MonoPtr, Coeff>;

struct Polynomial {
    std::vector<Term> terms;           // sorted by monomial pointer ordering
    void addTerm(const MonoPtr& m, Coeff c);
};

enum class Cmp : std::uint8_t { EQ0, GE0 };

struct Constraint {
    Polynomial poly;
    Cmp        cmp = Cmp::GE0;
    std::vector<std::pair<std::size_t,std::size_t>> neq;   // var‑var distinctness
};

struct Spot { std::size_t termIdx, monoIdx, argIdx; };


};