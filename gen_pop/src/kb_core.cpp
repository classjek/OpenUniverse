#include "kb_core.h"
#include <iostream>

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
    // handle zero atom (used to build zero monomial)
    if (rel.empty() && args.empty()) return true;  // empty atom is less than any other
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
    // if (items.size() == 1 && (*items[0].first).rel.empty()){
    //     std::cout << "Trying to print zero monomial bruh" << std::endl;
    // }
    for (const auto& [ap, e] : items) {
        if (!out.empty()) out += '*';
        out += ap->toString();
        if (e > 1) out += "^" + std::to_string(e);
    }
    return out.empty() ? "1" : out;  // empty monomial is 1
}
std::shared_ptr<Monomial> Monomial::zeroMon(){
    auto m = std::make_shared<Monomial>();
    m->items.emplace_back(std::make_shared<Atom>(), 1); // empty atom with exponent 0
    return m;
}
bool Monomial::isZero() const {
    // a monomial is zero if it contains an empty atom with exponent 0
    return items.size() == 1 && items[0].first->rel.empty();
}
std::shared_ptr<Monomial> Monomial::fromAtom(const AtomPtr& a) {
    auto m = std::make_shared<Monomial>();
    m->items.emplace_back(a, 1);
    return m;
}

std::shared_ptr<Monomial> Monomial::multiply(const std::shared_ptr<Monomial>& A, const std::shared_ptr<Monomial>& B)
{
    // if A or B is zero monomial, then just return the other
    if (A->isZero()) return B;
    if (B->isZero()) return A; 
    // else
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

// Polynomial Helpers //
// 
void Polynomial::canonicalise() {
    std::sort(terms.begin(), terms.end(),
        [](const Term& a, const Term& b){ return *(a.first) < *(b.first); });
    std::vector<Term> tmp;
    for (const auto& it : terms) {
        if (!tmp.empty() && *(tmp.back().first) == *(it.first))
            tmp.back().second += it.second;        // same monomial -> add coefficients
        else
            tmp.push_back(it);
    }
    terms.swap(tmp);
}
// Create a polynomial from a single monomial 
std::shared_ptr<Polynomial> Polynomial::fromMonomial(const MonoPtr& m) {
    auto p = std::make_shared<Polynomial>();
    p->terms.emplace_back(m, 1);
    return p;
}
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
    canonicalise();  // ensure polynomial is in canonical form after adding
}
std::string Polynomial::toString() const{
    std::string out; 
    for (const auto& [m,c] : terms) {
        if (!out.empty()) out += " + ";
        if(m->isZero()) { // if printing zero monomial
            out += std::to_string(c); // just print coeff
            continue;
        }
        if (c == 1) {
            out += m->toString();  
        } else if (c == -1) {
            out += '-' + m->toString();  
        } else {
            out += std::to_string(c) + '*' + m->toString(); 
        } 
    }
    return out; 
}

}