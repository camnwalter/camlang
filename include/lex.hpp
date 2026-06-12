#pragma once

#include "exceptions.hpp"
#include "token.hpp"

#include <string>
#include <utility>
#include <vector>

class Lexer {
private:
    std::string_view input;
    size_t index {0};
    size_t len;
    std::vector<Token> tokens;
    size_t line {1};
    size_t column {1};

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

        return input.at(index);
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
    explicit Lexer(std::string_view _input) :
        input(_input),
        len(input.length()) {}

    std::vector<Token> lex() {
        while (!isAtEnd()) {
            gettoken();
        }

        tokens.push_back({.tokenType = TokenType::Eof,
                          .lexeme = "",
                          .literal = {},
                          .line = line,
                          .column = column});

        std::cout << "Finished lexing" << std::endl;

        return tokens;
    }
};
