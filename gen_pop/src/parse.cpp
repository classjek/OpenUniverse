#include "parse.h"
#include <cctype>
#include <stdexcept>
#include <iostream>
#include <utility>

namespace kb::parse {

// Tokenizer
enum class Tok { IDENT, NUMBER, PLUS, MINUS, STAR, LP, RP, COMMA,
                 GE, EQ, NEQ, COLON, END };

struct Token { Tok kind; std::string text; };

class Lexer {
public:
    explicit Lexer(const std::string &s) : src(s), p(src.c_str()) { next(); }

    const Token &peek() const { return cur; }
    Token        pop()  { Token t = cur; next(); return t; }
    Token peekNext() const {
        const char *saveP = p;
        Token saveCur     = cur;
        // create a throw-away lexer just to advance once
        Lexer tmp(*this);
        tmp.pop();                 // skip current token
        Token nxt = tmp.peek();
        return nxt;
    }

private:
    void next() {
        while (std::isspace(*p)) ++p;
        if (*p == '\0') { cur = {Tok::END, ""}; return; }
        char c = *p;
        if (std::isalpha(c) || c=='_') {
            const char *start = p;
            while (std::isalnum(*p) || *p=='_') ++p;
            cur = {Tok::IDENT, std::string(start, p)}; return;
        }
        if (std::isdigit(c)) {
            const char *start = p;
            while (std::isdigit(*p)) ++p;
            cur = {Tok::NUMBER, std::string(start, p)}; return;
        }
        ++p; // consume single char tokens
        switch (c) {
            case '+': cur = {Tok::PLUS, "+"}; return;
            case '-': cur = {Tok::MINUS,"-"}; return;
            case '*': cur = {Tok::STAR, "*"}; return;
            case '(': cur = {Tok::LP,   "("}; return;
            case ')': cur = {Tok::RP,   ")"}; return;
            case ',': cur = {Tok::COMMA,","}; return;
            case ':': cur = {Tok::COLON, ":"}; return;
            case '>': if (*p=='='){ ++p; cur={Tok::GE, ">="}; return; }; break;
            case '!': if (*p=='='){ ++p; cur = {Tok::NEQ, "!="}; return; } break;
            default:  break;
        }
        if (c=='=') { cur={Tok::EQ, "="}; return; }
        throw std::runtime_error("Unexpected char in input");
    }

    const std::string &src; const char *p; Token cur;
};


struct Parser {
    explicit Parser(const std::string &s) : lex(s) {}
    Constraint parse();
private:
    // grammar helpers
    Polynomial parseSum();              // sum  ::= product ((+|-) product)*
    std::pair<bool,Coeff>  parseCoefficient();      // coefficient ::= number? '*'
    MonoPtr    parseProduct();          // product ::= factor (factor)*
    MonoPtr    parseFactor();           // factor ::= atom | number
    AtomPtr    parseAtom();

    // utility
    bool accept(Tok k){ if(lex.peek().kind==k){ lex.pop(); return true;} return false; }
    void expect(Tok k,const char*msg){ if(!accept(k)) throw std::runtime_error(msg);}  
    std::size_t varIndex(const std::string& name, std::vector<std::string>& vars);

    Lexer lex;
};

std::size_t varIndex(const std::string& name, std::vector<std::string>& vars) {
    auto it = std::find(vars.begin(), vars.end(), name);
    if (it != vars.end()) return std::distance(vars.begin(), it);
    vars.push_back(name);
    return vars.size()-1;
}

AtomPtr Parser::parseAtom() {
    Token id = lex.pop();               
    expect(Tok::LP, "Expected '('");
    std::vector<Sym> args;
    if (lex.peek().kind != Tok::RP) {
        do {
            if (lex.peek().kind != Tok::IDENT)
                throw std::runtime_error("Expected identifier in arg list");
            args.push_back(lex.pop().text);
        } while (accept(Tok::COMMA));
    }
    expect(Tok::RP, "Expected ')'");
    auto a = std::make_shared<Atom>();
    a->rel = id.text; a->args = std::move(args);
    return a;
}

MonoPtr Parser::parseFactor() {
    if (lex.peek().kind == Tok::IDENT) {
        return Monomial::fromAtom(parseAtom());
    }
    if (lex.peek().kind == Tok::NUMBER) {
        // treat numeric constant n as n * 1, handled in Polynomial layer
        int val = std::stoi(lex.pop().text);
        auto one = std::make_shared<Monomial>(); // empty product == 1
        auto monoPtr = one;
        Polynomial dummy;
        dummy.addTerm(monoPtr, val);            // store constant in dummy
        // pull the monomial pointer back out (same obj) for caller to use
        return monoPtr;                         // coeff handled in parseProduct
    }
    // throw std::runtime_error("Unexpected token in factor");
    throw std::runtime_error(
        "Unexpected token in factor: kind=" +
        std::to_string(static_cast<int>(lex.peek().kind)) +
        " text='" + lex.peek().text + "'");
}

std::pair<bool, Coeff> Parser::parseCoefficient(){
    // no explicit coefficient, ex: Q(x,y)
    if (lex.peek().kind != Tok::NUMBER) { 
        return std::make_pair(true, 1); // attach coefficient 1
    }
    // explicit coefficient, ex: 3 or 3*Q(x,y)
    std::string numStr = lex.pop().text;
    if (lex.peek().kind != Tok::STAR) { // Check for following term(t) or if is constant(f)
        // no '*' after number, so just return the number, ex: 3
        return std::make_pair(false, static_cast<Coeff>(std::stoll(numStr)));
    }
    // else we have a following monomial, ex: 3*Q(x,y)
    accept(Tok::STAR); // process *
    return std::make_pair(true, static_cast<Coeff>(std::stoll(numStr)));
}

MonoPtr Parser::parseProduct() {
    auto m = parseFactor();
    // Implicit multiplication: IDENT IDENT … or with '*'
    while (lex.peek().kind == Tok::IDENT || lex.peek().kind == Tok::LP || lex.peek().kind == Tok::STAR) {
        if (accept(Tok::STAR)) continue;        // consume explicit '*'
        auto rhs = parseFactor();
        m = Monomial::multiply(m, rhs);
    }
    return m;
}

Polynomial Parser::parseSum() {
    Polynomial P;
    bool neg = false; // handle optional leading sign (+/-)
    if (accept(Tok::PLUS) || (neg = accept(Tok::MINUS))) {}
    
    // parse first object and add its monomial to polynomial
    auto coef = parseCoefficient();
    if (coef.first){ // there is following term after coeff
        auto firstMono = parseProduct(); // process term
        P.addTerm(firstMono, neg ? -1 * coef.second : 1 * coef.second);
    } else { // no following term 
        // add zeroMonomial to represent constant 
        P.addTerm(Monomial::zeroMon(), neg ? -1 * coef.second : 1 * coef.second);
    }

    // add remaining terms in the sum
    while (lex.peek().kind == Tok::PLUS || lex.peek().kind == Tok::MINUS) {
        neg = accept(Tok::MINUS); 
        if (!neg) accept(Tok::PLUS);
        auto coef = parseCoefficient(); 
        if (coef.first){ // there is term after coeff
            auto m = parseProduct(); // process term 
            P.addTerm(m, neg ? -1 * coef.second : 1 * coef.second);
        } else { // no following term
            P.addTerm(Monomial::zeroMon(), neg ? -1 * coef.second : 1 * coef.second);
        }
    }
    return P;
}

Constraint Parser::parse() {
    Constraint C;
    // handle distinctness guard like x != y : 
    std::vector<std::string> vars;               // keeps order of seen variables

    // Handle distinctness guard: x != y
    if (lex.peek().kind == Tok::IDENT && lex.peekNext().kind == Tok::NEQ) {
        do {
            Token a = lex.pop();                // IDENT
            expect(Tok::NEQ, "need '!=' in guard");
            Token b = lex.pop();                // IDENT

            C.neq.emplace_back(a.text, b.text);

        } while (accept(Tok::COMMA));
        expect(Tok::COLON, "missing ':' after guard");
    }

    // handles the left hand side of the constraint, generating its polynomial representation
    Polynomial lhs = parseSum();

    Tok compTok = lex.peek().kind;
    if (compTok == Tok::GE || compTok == Tok::EQ) lex.pop();
    else throw std::runtime_error("Expected '>=' or '='");

    Polynomial rhs = parseSum();

    // Move rhs to lhs just in case 
    for (auto [m,c] : rhs.terms) lhs.addTerm(m, -c);
    C.poly = std::move(lhs);
    C.cmp  = (compTok == Tok::EQ ? Cmp::EQ0 : Cmp::GE0);

    if (lex.peek().kind != Tok::END)
        throw std::runtime_error("Unexpected trailing tokens");

    return C;
}

// Public API
Constraint parseConstraint(const std::string &text) {
    Parser p(text);
    return p.parse();
}

}