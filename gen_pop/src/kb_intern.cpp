#include "kb_intern.h"

namespace kb::intern {

// actual storage (one definition)
std::unordered_map<std::string, AtomPtr> atomPool;
std::unordered_map<std::string, MonoPtr> monomialPool;

// canonical-string builder (same as your makeKey)
static std::string atomKey(const std::string& rel,
                           const std::vector<Sym>& args)
{
    std::string k = rel;
    k.push_back('(');
    for (std::size_t i=0;i<args.size();++i) {
        k += args[i];
        if (i+1<args.size()) k.push_back(',');
    }
    k.push_back(')');
    return k;
}

AtomPtr internAtom(const std::string& rel, const std::vector<Sym>& args)
{
    std::string key = atomKey(rel, args);
    auto [it, inserted] = atomPool.try_emplace(key, nullptr);
    if (inserted) {
        auto a = std::make_shared<Atom>();
        a->rel  = rel;
        a->args = args;
        it->second = a;
    }
    return it->second;
}

}
