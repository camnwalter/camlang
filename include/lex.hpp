#pragma once

#include "exceptions.hpp"
#include "token.hpp"

#include <string>
#include <vector>

class Lexer {
private:
    const std::string input;
    size_t index;
    size_t len;
    std::vector<Token*> tokens;
    uint32_t lineno;
    uint32_t colno;

private:
    void newline() {
        colno = 1;
        lineno++;
    }

    bool isAlpha(char c) {
        return isalpha(c) || c == '_';
    }

    char peek() {
        if (index >= len) {
            return 0;
        }

        return input[index];
    }

    void next() {
        index++;
        colno++;
    }

    char consume() {
        char c = peek();
        next();
        return c;
    }

    void whitespace();
    void comment();
    void number();
    void identifier();
    void string();
    void special();

    void gettoken() {
        whitespace();

        char c = peek();
        if (isdigit(c)) {
            number();
        } else if (isAlpha(c)) {
            identifier();
        } else if (c == '"') {
            string();
        } else {
            special();
        }
    }

public:
    Lexer(std::string _input) :
        input(_input), index(0), len(_input.length()), lineno(1), colno(1) {}

    std::vector<Token*> lex() {
        if (index >= len) {
            // this can only happen if we try to call lex() more than once, so
            // we can just noop
            return tokens;
        }

        while (index < len) {
            gettoken();
        }

        tokens.push_back(new Token(TokenType::Eof, "", lineno, colno));

        std::cout << "Finished lexing" << std::endl;

        return tokens;
    }
};
