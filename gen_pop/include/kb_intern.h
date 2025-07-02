#pragma once
#include <unordered_map>
#include "kb_core.h" 

namespace kb::intern {
extern std::unordered_map<std::string, AtomPtr> atomPool;
extern std::unordered_map<std::string, MonoPtr> monomialPool;

AtomPtr   internAtom(const std::string& rel, const std::vector<Sym>& args);
MonoPtr   internMonomial(const std::vector<AtomPtr>& atoms);
}