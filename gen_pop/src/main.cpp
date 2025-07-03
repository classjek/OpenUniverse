#include "duckdb.hpp"
#include "kb_core.h"
#include "parse.h"
#include "equivalence.h"
#include "kb_intern.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <filesystem>
#include <string_view>

int main(){
    //using namespace duckdb; 
    using namespace kb;
    using namespace kb::parse; 
    using namespace kb::eq;
    using kb::intern::atomPool;

    namespace fs = std::filesystem;

    std::ifstream in("../data/constraints.txt");      // adjust path if needed
    if (!in) {
        std::cerr << "Could not open constraints file\n";
        return 1;
    }

    std::unordered_set<std::string> groundVariables;
    std::vector<Constraint> constraints;
    std::string line;

    while (std::getline(in, line)) {
        if (line.empty() || (line[0] == '#' && line[1] == '#')) continue;    
        if (line[0] == ';') {
            groundVariables.insert(line.substr(1));
            continue;
        }      
        try {
            // Only add constraint if not already present
            auto constraint = parse::parseConstraint(line);
            if (std::find(constraints.begin(), constraints.end(), constraint) == constraints.end()) {
                constraints.push_back(constraint);
            }
        } catch (const std::exception& e) {
            std::cerr << "Parse error in line: \"" << line << "\"\n  " << e.what() << '\n';
        }
    }
    // // Display constraints
    // std::cout << '\n' << "Printing all " << constraints.size() << " Constraints:" << std::endl;
    // for (const auto& c : constraints) {
    //     std::cout << c.poly.toString() << " " << (c.cmp == Cmp::GE0 ? ">=" : "=") << " 0\n";
    //     // if (!c.neq.empty()) {
    //     //     std::cout << "  Distinctness guard: ";
    //     //     for (const auto& [a, b] : c.neq) {
    //     //         std::cout << a << " != " << b << ", ";
    //     //     }
    //     //     std::cout << '\n';
    //     // }
    // }

    // Test out genMonomialMap function
    // std::cout << '\n' << "Testing genMonomialMap" << std::endl;
    // auto monoMap = kb::eq::genMonomialMap(constraints);
    // for (const auto& [sig, monoPtr] : monoMap) {
    //     std::cout << sig << "  ->  " << monoPtr->toString() << '\n';
    // }

    // std::cout << '\n' << "Testing Equivalences Between Monomials That Differ in Ground Variables" << std::endl;
    // auto mymap = computeGroundNameClasses(constraints, groundVariables);

    // std::cout << "===== canonical atom pool =====\n";
    // for (const auto& [key, atomPtr] : atomPool) {
    //     // key is the canonical string "Rel(arg1,arg2,...)"
    //     // atomPtr is the shared_ptr<Atom> that every monomial re-uses
    //     std::cout << key << "   @ptr=" << atomPtr.get() << '\n';
    // }
    // std::cout << "================================\n";

    // std::cout << "Testing Mapping Function" << std::endl;
    std::unordered_map<Sym, std::string> realMap = relVarMap(constraints);
    // for (const auto& [sym, var] : realMap) {
    //     std::cout << sym << " -> " << var << '\n';
    // }

    // Display constraints
    std::cout << '\n' << "Printing all " << constraints.size() << " Constraints:" << std::endl;
    for (const auto& c : constraints) {
        std::cout << c.poly.toString() << " " << (c.cmp == Cmp::GE0 ? ">=" : "=") << " 0\n";
        //std::cout << c.poly.toStringWithMap(realMap) << " " << (c.cmp == Cmp::GE0 ? ">=" : "=") << " 0\n" << '\n';
    }

    // Generate .gms constraint strings
    std::unordered_set<std::string> gmsConstraints;
    for (const auto& constraint : constraints){
        std::string gmsStr = constraint.poly.toStringWithMap(realMap);
        if (constraint.cmp == Cmp::GE0) {
            gmsStr += " =g= 0 ;" ;
        } else {
            gmsStr += " =e= 0 ;";
        }
        gmsConstraints.insert(gmsStr);
    }

    std::cout << "===== .gms file =====" << std::endl;
    // Variables Line
    std::string varLine = "Variables ";
    for (const auto& [sym, var] : realMap) {
        varLine += var + ", ";
    }
    // Equation Line and Equations
    std::string eqLine = "Equations ";
    std::string eqs; 
    int idx = 1; 
    for (const auto& gconstraint : gmsConstraints){
        eqLine += "c" + std::to_string(idx) + ", ";
        eqs += "c" + std::to_string(idx++) + ".. " + gconstraint + '\n';
        //std::cout << "c" + std::to_string(idx++) + ".. " << gconstraint << '\n';
    }
    std::cout << std::string_view(varLine.data(), varLine.size()-2) << " ;" << std::endl;
    std::cout << std::string_view(eqLine.data(), eqLine.size()-2) << " ;" << std::endl;
    std::cout << eqs << std::endl;

    // Write to .gms file
    std::ofstream out("problem.gms");
    if (!out) {
        std::cerr << "Error: cannot open output file\n";
        return 1;
    }
    out << std::string_view(varLine.data(), varLine.size()-2) << " ;\n";
    out << std::string_view(eqLine.data(), eqLine.size()-2) << " ;\n";
    out << eqs;
    
    out.close();
    std::cout << "Wrote problem.gms successfully" << std::endl;

    // Generate equivalence map
    // equiv_map = gen_equiv_map(constraints)
    // call function to calculate quantifier rank
    // call ground function to generate grounding: ground(constraints, quantRank)
    // need to make sure groundings take into account g_0 != g_1
    // another function to write this to a .gms file, keeping track of 
    // what monomials go to which variables 



    // fs::path parquet = fs::current_path() / "../data/yago3-10.parquet";
    // std::cout << "parquet path = " << parquet << '\n';
    // if (!fs::exists(parquet)) {
    //     std::cerr << "ERROR: Parquet file not found at that path\n";
    //     return 1;
    // }

    // // Start in-memory DuckDB instance
    // DuckDB db(nullptr);  //  pass nullptr so DB doesn't persist on disk
    // Connection conn(db);

    // conn.Query("INSTALL parquet;");
    // conn.Query("LOAD parquet;"); 

    // std::string sql =
    // "CREATE OR REPLACE TABLE yago AS "
    // "SELECT * FROM read_parquet('" + parquet.string() + "');";
    // auto res = conn.Query(sql);
    // if (res->HasError()) {
    //     std::cerr << res->GetError() << '\n';
    //     return 1; 
    // } else {
    //     std::cout << "Table 'yago' created successfully.\n";
    // }

    // Initialize atoms q and r
    // auto q = std::make_shared<Atom>();
    // q->rel  = "Q";
    // q->args = {"x", "b"};
    // auto r = std::make_shared<Atom>();
    // r->rel = "R";
    // r->args = {"x", "y", "z"};
    // auto test_m = Monomial::zeroMon(); 
    // auto m1 = Monomial::fromAtom(q);
    // // poly stuff
    // auto p1 = Polynomial::fromMonomial(test_m);
    // auto p2 = Polynomial::fromMonomial(m1);

    // p1->addTerm(test_m, 3);
    // p1->addTerm(m1, -2);
    // p1->addTerm(test_m, 4);
    // std::cout << " Result3 poly: " << p1->toString() << '\n';

    // auto head = conn.Query("SELECT * FROM yago LIMIT 10;");
    // for (auto &name : head->names) std::cout << name << '\t';
    // std::cout << '\n' << '\n';
    // for (auto &row : *head) {
    //     for (idx_t i = 0; i < head->ColumnCount(); ++i)
    //         std::cout << row.GetValue<std::string>(i) << '\t';
    //     std::cout << '\n';
    // }

    return 0;
}