#pragma once

#include <variant>
#include <string>
#include <iostream>
#include <cassert>
#include <unordered_map>
#include <memory>

enum TokenType
{
    eof = -1,

    PLUS,
    MINUS,
    STAR,
    SLASH,
    CARET,

    NOT,
    LT,
    LE,
    GT,
    GE,
    EQ,
    NE,

    SEMICOLON,
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,

    IDENTIFIER,
    STRING,
    NUMBER,
    TRUE,
    FALSE,
};

std::unordered_map<std::string, TokenType> textToTokenType()
{

    static std::unordered_map<std::string, TokenType> map = {
        {"true", TRUE},
        {"false", FALSE},
    };
    return map;
}

struct Token
{
    TokenType tt;
    std::variant<bool, double, std::string> value;
    std::string lexeme; // stringified value
    uint32_t lineno;
    uint32_t colno;

    // This is a work-around for bools
    Token(TokenType t, int v, uint32_t line, uint32_t col)
        : tt(t),
          value(static_cast<bool>(v)),
          lexeme(std::to_string(v)),
          lineno(line),
          colno(col)
    {
    }

    Token(TokenType t, double v, uint32_t line, uint32_t col)
        : tt(t),
          value(v),
          lexeme(std::to_string(v)),
          lineno(line),
          colno(col)
    {
    }

    Token(TokenType t, std::string v, uint32_t line, uint32_t col)
        : tt(t),
          value(v),
          lexeme(v),
          lineno(line),
          colno(col)
    {
    }

    void print()
    {
        std::cout << "Token(" << tt << ": value=" << lexeme << ", [" << lineno << ":" << colno << "])" << std::endl;
    }
};

using TokenPtr = std::shared_ptr<Token>;
