#include "duckdb.hpp"
#include "kb_core.h"
#include "parse.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <filesystem>

int main(){
    //using namespace duckdb; 
    using namespace kb;
    using namespace kb::parse; 

    namespace fs = std::filesystem;
    
    std::cout << "Hello there guy, " << __cplusplus << std::endl;
    std::cout << "cwd  = " << fs::current_path() << '\n';

    std::ifstream in("../data/constraints.txt");      // adjust path if needed
    if (!in) {
        std::cerr << "Could not open constraints file\n";
        return 1;
    }

    // Working on constraint parsing
    std::vector<std::string> groundVariables; 
    std::vector<Constraint> constraints;
    std::string line;

    // maybe change parseConstraint to take in groundVariables and then replace all generics
    // with x_1, x_2 whatever, ordered per monomial
    // unless we have some constraint, which could be tricky
    // store atoms and monomials in maps
    // potentially store polynomials in some lookup structure 
    // check funky characters
    // implement those annoying constraints 
    // ^ think about how I will do grounding before doing this
    // Change symID thing? 
    while (std::getline(in, line)) {
        if (line.empty() || (line[0] == '#' && line[1] == '#')) continue;    
        if (line[0] == ';') {
            groundVariables.push_back(line.substr(1));
            continue;
        }              
        try {
            constraints.push_back(parse::parseConstraint(line));
        } catch (const std::exception& e) {
            std::cerr << "Parse error in line: \"" << line << "\"\n  " << e.what() << '\n';
        }
    }
    std::cout << '\n' << "How many constraints? " << constraints.size() << std::endl;
    for (const auto& c : constraints) {
        std::cout << "Constraint: " << c.poly.toString() << " " 
                  << (c.cmp == Cmp::GE0 ? ">=" : "=") << " 0\n";
    }

    std::cout << "Ground Variables considered:" << std::endl;
    for (const auto& gv: groundVariables){
        std::cout << gv;
        if (gv != groundVariables.back()) std::cout << ", ";
    }



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