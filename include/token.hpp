#pragma once

#include <cstdint>
#include <iostream>
#include <string>
#include <unordered_map>
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

    // Assignments
    Eq,
    PlusEq,
    MinusEq,
    StarEq,
    SlashEq,

    // Delimiters
    Comma,
    Semi,
    Colon,
    LParen,
    RParen,
    LBrace,
    RBrace,

    // Control Flow
    If,
    Else,
    While,
    Return,

    // Literals
    Identifier,
    String,
    FNumber,
    INumber,
    True,
    False,

    // Declarations
    Var,
    Const,
    Fn,
};

extern const std::unordered_map<std::string_view, TokenType> reservedKeywords;

using Literal
    = std::variant<std::monostate, bool, uint32_t, double, std::string>;

struct Token {
    TokenType tokenType;
    std::string_view lexeme;
    Literal literal;
    size_t line;
    size_t column;
};
