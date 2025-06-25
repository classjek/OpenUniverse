#include "kb_core.h"

namespace kb {
// Atom Helpers
std::string Atom::toString() const{
    std::string out = rel + '(';
    for (std::size_t i = 0; i < args.size(); ++i) {
        if (i) out += ',';
        out += args[i];
    }
    out += ')';
    return out;
}

bool Atom::operator<(const Atom& o) const noexcept {
    return std::tie(rel, args) < std::tie(o.rel, o.args);
}
bool Atom::operator==(const Atom& o) const noexcept {
    return rel == o.rel && args == o.args;
}


// Monomial Helpers
void Monomial::canonicalise() {
    std::sort(items.begin(), items.end(),
        [](const MonoItem& a, const MonoItem& b){ return *(a.first) < *(b.first); });
    std::vector<MonoItem> tmp;
    for (const auto& it : items) {
        if (!tmp.empty() && *(tmp.back().first) == *(it.first))
            tmp.back().second += it.second;           // same atom → add exponents
        else
            tmp.push_back(it);
    }
    items.swap(tmp);
}

std::string Monomial::toString() const {
    std::string out;
    for (const auto& [ap, e] : items) {
        if (!out.empty()) out += '*';
        out += ap->toString();
        if (e > 1) out += "^" + std::to_string(e);
    }
    return out.empty() ? "1" : out;  // empty monomial is 1
}

std::shared_ptr<Monomial> Monomial::fromAtom(const AtomPtr& a) {
    auto m = std::make_shared<Monomial>();
    m->items.emplace_back(a, 1);
    return m;
}

std::shared_ptr<Monomial> Monomial::multiply(const std::shared_ptr<Monomial>& A, const std::shared_ptr<Monomial>& B)
{
    auto m = std::make_shared<Monomial>();
    m->items.reserve(A->items.size() + B->items.size());
    m->items.insert(m->items.end(), A->items.begin(), A->items.end());
    m->items.insert(m->items.end(), B->items.begin(), B->items.end());
    m->canonicalise();
    return m;
}

std::vector<AtomPtr> Monomial::expandedAtoms() const {
    std::vector<AtomPtr> out;
    for (const auto& [ap, e] : items)
        for (Exponent k = 0; k < e; ++k) out.push_back(ap);
    return out;
}

bool Monomial::operator<(const Monomial& o) const noexcept {
    return items < o.items;
}
bool Monomial::operator==(const Monomial& o) const noexcept {
    return items == o.items;
}

// Polynomial Helpers
void Polynomial::addTerm(const MonoPtr& m, Coeff c) {
    if (c == 0) return;
    auto cmp = [](const Term& a, const Term& b){ return *(a.first) < *(b.first); };
    auto it  = std::lower_bound(terms.begin(), terms.end(), Term{m,0}, cmp);
    if (it != terms.end() && !(*(m) < *(it->first))) {
        it->second += c;
        if (it->second == 0) terms.erase(it);
    } else {
        terms.insert(it, {m, c});
    }
}

}