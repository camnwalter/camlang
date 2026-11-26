#include "../include/lex.hpp"
#include "../include/parse.hpp"
#include "../include/print.hpp"
#include "../include/type.hpp"

#include <fstream>
#include <iostream>
#include <print>
#include <sstream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Expected 1 argument, got " << (argc - 1) << std::endl;
    }

    std::ifstream t(argv[1]);
    std::stringstream buffer;
    buffer << t.rdbuf();

    Lexer lexer(buffer.str());
    auto toks = lexer.lex();

    Parser parser(std::move(toks));
    auto file = parser.parse();

    Printer {}(file);

    TypeChecker {}(file);

    return 0;
}
