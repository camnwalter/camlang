#pragma once

#include <variant>
#include <string>
#include <iostream>
#include <cassert>
#include <unordered_map>

[[noreturn]] void TODO(const char *text)
{
    std::cerr << "Todo: " << text << std::endl;
    assert(false);
}

enum TokenType
{
    eof = -1,

    PLUS,
    MINUS,
    TIMES,
    DIVIDE,

    SEMICOLON,

    IDENTIFIER,
    STRING,
    NUMBER,
};

struct Token
{
    TokenType tt;
    std::variant<int32_t, std::string> value;
    std::string lexeme; // stringified value

    Token(TokenType t, int32_t v) : tt(t), value(v), lexeme(std::to_string(v))
    {
    }

    Token(TokenType t, std::string v) : tt(t), value(v), lexeme(v)
    {
    }

    void print()
    {
        std::cout << "Token(" << tt << ": value=" << lexeme << ")" << std::endl;
    }
};
