#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include "lex.hpp"
#include "parse.hpp"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Expected 1 argument, got " << (argc - 1) << std::endl;
    }

    std::ifstream t(argv[1]);
    std::stringstream buffer;
    buffer << t.rdbuf();

    Lexer lexer(buffer.str());
    std::vector<Token *> toks = lexer.lex();

    for (auto &&i : toks)
    {
        i->print();
    }

    Parser parser(toks);
    AstNode *root = parser.parse();
    root->print();
    std::cout << std::endl;
    return 0;
}
