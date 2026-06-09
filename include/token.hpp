#pragma once

#include <cstdint>
#include <iostream>
#include <string>
#include <unordered_map>
#include <variant>

enum class TokenType {
    Eof,

    Plus,
    Minus,
    Star,
    Slash,
    Caret,
    Mod,

    Bang,
    LAngle,
    LAngleEq,
    RAngle,
    RAngleEq,
    EqEq,
    BangEq,

    LShift,
    RShift,
    Tilde,
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

extern std::unordered_map<std::string, TokenType> reservedKeywords;

using Literal =
    std::variant<std::monostate, bool, uint32_t, double, std::string>;

struct Token {
    TokenType tokenType;
    std::string lexeme;
    Literal literal;
    size_t line;
    size_t column;
};
