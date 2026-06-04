#include "../include/lex.hpp"
#include "../include/parse.hpp"

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

    for (auto&& i : toks) {
        i->print();
    }

    Parser parser(toks);
    auto root = parser.parse();
    std::println("{}", root->print());
    return 0;
}
