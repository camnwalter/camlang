#pragma once

#include "bimap.hpp"

#include <cstdint>
#include <iostream>
#include <string>
#include <variant>

enum class TokenType {
    Eof,
    Error,

    Plus,
    Minus,
    Star,
    Slash,
    Caret,
    Mod,

    Not,
    LT,
    LE,
    GT,
    GE,
    EqEq,
    Ne,

    LShift,
    RShift,
    BitNot,
    BitAnd,
    Xor,
    BitOr,
    And,
    Or,

    Eq,
    PlusEq,
    MinusEq,
    StarEq,
    SlashEq,

    Comma,
    Semi,
    LParen,
    RParen,
    LBrace,
    RBrace,

    If,
    Else,
    While,
    Fn,
    Return,

    Identifier,
    String,
    FNumber,
    INumber,
    Bool,
    True,
    False,

    Var,
    Const,
};

extern BiMap<TokenType, std::string> TokenTypes;

struct Token {
    TokenType tt;
    std::variant<bool, double, uint32_t, std::string> value;
    std::string lexeme; // stringified value
    uint32_t lineno;
    uint32_t colno;

    Token(TokenType t, uint32_t v, uint32_t line, uint32_t col) :
        tt(t), value(v), lexeme(std::to_string(v)), lineno(line), colno(col) {}

    Token(TokenType t, double v, uint32_t line, uint32_t col) :
        tt(t), value(v), lexeme(std::to_string(v)), lineno(line), colno(col) {}

    Token(TokenType t, std::string v, uint32_t line, uint32_t col) :
        tt(t), value(v), lexeme(v), lineno(line), colno(col) {}

    void print() {
        std::cout << "Token(" << TokenTypes[tt] << ": value=" << lexeme << ", ["
                  << lineno << ":" << colno << "])" << std::endl;
    }
};
