#include "parse.h"
#include <cctype>
#include <stdexcept>
#include <iostream>
#include <utility>

namespace kb::parse {

// TODO: Also want to allow the user to indicate variables that
// they want to consider 
// and some way to indicate which variables are generics

// Tokenizer
enum class Tok { IDENT, NUMBER, PLUS, MINUS, STAR, LP, RP, COMMA,
                 GE, EQ, END };

struct Token { Tok kind; std::string text; };

class Lexer {
public:
    explicit Lexer(const std::string &s) : src(s), p(src.c_str()) { next(); }

    const Token &peek() const { return cur; }
    Token        pop()  { Token t = cur; next(); return t; }

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
            case '>': if (*p=='='){ ++p; cur={Tok::GE, ">="}; return; }
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

    Lexer lex;
};

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
        return std::make_pair(true, 1); 
    }
    // explicit coefficient, ex: 3*Q(x,y)
    // check if there is a following term(true) or it is a constant(false)
    // ex: 3 or 3*Q(x,y)
    std::string numStr = lex.pop().text;
    if (lex.peek().kind != Tok::STAR) {
        // std::cout << "This is a constant: " << static_cast<Coeff>(std::stoll(numStr)) << std::endl;
        // no '*' after number, so just return the number
        return std::make_pair(false, static_cast<Coeff>(std::stoll(numStr)));
    }
    // else we have a following monomial
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

    // handle optional leading sign (+/-)
    bool neg = false;
    if (accept(Tok::PLUS) || (neg = accept(Tok::MINUS))) {}
    // parse first object and add its monomial to polynomial
    auto coef = parseCoefficient();
    // have parseCoefficient return a bool and the coeff 
    // if bool is false, then skip firstMono and create a zeroMono
    // otherwise, use parseproduct()
    // auto firstMono = parseProduct();
    if (coef.first){ // there is following term after coeff
        auto firstMono = parseProduct(); // process term
        P.addTerm(firstMono, neg ? -1 * coef.second : 1 * coef.second);
    } else { // no following term 
        // add zeroMonomial to represent constant 
        P.addTerm(Monomial::zeroMon(), neg ? -1 * coef.second : 1 * coef.second);
    }
    // P.addTerm(firstMono, neg ? -1 * coef : 1 * coef);
    // this is just for easy testing of parseCoefficient()
    //P.addTerm(firstMono, neg ? -1 : 1);

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
        //auto m = parseProduct();
        // add back, same as above 
        // P.addTerm(m, neg ? -1 * coef : 1 * coef);
        //P.addTerm(m, neg ? -1 : 1);
    }
    return P;
}

Constraint Parser::parse() {
    Constraint C;
    // handles the left hand side of the constraint, generating its polynomial representation
    Polynomial lhs = parseSum();
    std::cout << "Parsed LHS: " << lhs.toString() << std::endl;

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