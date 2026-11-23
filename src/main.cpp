#include "../include/lex.hpp"
#include "../include/parse.hpp"
#include "../include/print.hpp"

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

    // We store text into a variable so that string_views work easily
    const auto text = buffer.str();
    Lexer lexer(text);
    auto toks = lexer.lex();

    Parser parser(std::move(toks));
    auto file = parser.parse();

    Printer {}(file);

    return 0;
}
