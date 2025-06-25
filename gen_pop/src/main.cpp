#include "duckdb.hpp"
#include "kb_core.h"
#include <iostream>
#include <vector>
#include <filesystem>

int main(){
    using namespace duckdb; 
    using namespace kb;

    namespace fs = std::filesystem;
    
    std::cout << "Hello there guy, " << __cplusplus << std::endl;
    std::cout << "cwd  = " << fs::current_path() << '\n';

    fs::path parquet = fs::current_path() / "../data/yago3-10.parquet";
    std::cout << "parquet path = " << parquet << '\n';
    if (!fs::exists(parquet)) {
        std::cerr << "ERROR: Parquet file not found at that path\n";
        return 1;
    }

    // Start in-memory DuckDB instance
    DuckDB db(nullptr);  //  pass nullptr so DB doesn't persist on disk
    Connection conn(db);

    conn.Query("INSTALL parquet;");
    conn.Query("LOAD parquet;"); 

    std::string sql =
    "CREATE OR REPLACE TABLE yago AS "
    "SELECT * FROM read_parquet('" + parquet.string() + "');";
    auto res = conn.Query(sql);
    if (res->HasError()) {
        std::cerr << res->GetError() << '\n';
        return 1; 
    } else {
        std::cout << "Table 'yago' created successfully.\n";
    }

    // Initialize atoms q and r
    auto q = std::make_shared<Atom>();
    q->rel  = "Q";
    q->args = {"x", "b"};
    auto r = std::make_shared<Atom>();
    r->rel = "R";
    r->args = {"x", "y", "z"};

    // Initialize Monomials from these atoms
    auto m1 = Monomial::fromAtom(q);
    auto m2 = Monomial::fromAtom(r);
    auto m3 = Monomial::multiply(m1,m2); 
    m3 = Monomial::multiply(m3, m2); 
    m3 = Monomial::multiply(m3, m1); 

    // Initialize Polynomial from monomial
    auto p1 = Polynomial::fromMonomial(m1);
    p1->addTerm(m2, -4);
    p1->addTerm(m2, 2);
    p1->addTerm(m3, -1);
    std::cout << p1->toString() << std::endl;

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