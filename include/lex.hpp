#pragma once

#include "exceptions.hpp"
#include "token.hpp"

#include <string>
#include <vector>

class Lexer {
private:
    std::string input;
    size_t index;
    size_t len;
    std::vector<Token> tokens;
    size_t line;
    size_t column;

private:
    void newline() {
        column = 1;
        line++;
    }

    bool isAlpha(char c) {
        return isalpha(c) || c == '_';
    }

    bool isAtEnd() {
        return index >= len;
    }

    char peek() {
        if (isAtEnd()) {
            return 0;
        }

        return input[index];
    }

    void next() {
        index++;
        column++;
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

        if (isAtEnd()) {
            return;
        }

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
    explicit Lexer(std::string _input) :
        input(_input),
        index(0),
        len(input.length()),
        line(1),
        column(1) {}

    std::vector<Token> lex() {
        if (isAtEnd()) {
            // this can only happen if we try to call lex() more than once, so
            // we can just noop
            return tokens;
        }

        while (!isAtEnd()) {
            gettoken();
        }

        tokens.push_back({TokenType::Eof, "", {}, line, column});

        std::cout << "Finished lexing" << std::endl;

        return tokens;
    }
};
