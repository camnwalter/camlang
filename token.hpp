#pragma once

#include "bimap.hpp"

#include <cassert>
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

BiMap<TokenType, std::string> TokenTypes = {
    { TokenType::Eof, std::nullopt },
    { TokenType::Error, std::nullopt },
    { TokenType::Plus, "+" },
    { TokenType::Minus, "-" },
    { TokenType::Star, "*" },
    { TokenType::Slash, "/" },
    { TokenType::Caret, "^" },
    { TokenType::Mod, "mod" },
    { TokenType::Not, "!" },
    { TokenType::LT, "<" },
    { TokenType::LE, "<=" },
    { TokenType::GT, ">" },
    { TokenType::GE, ">=" },
    { TokenType::EqEq, "==" },
    { TokenType::Ne, "!=" },
    { TokenType::LShift, "<<" },
    { TokenType::RShift, ">>" },
    { TokenType::BitNot, "~" },
    { TokenType::BitAnd, "&" },
    { TokenType::Xor, "xor" },
    { TokenType::BitOr, "|" },
    { TokenType::And, "and" },
    { TokenType::Or, "or" },
    { TokenType::EqEq, "=" },
    { TokenType::PlusEq, "+=" },
    { TokenType::MinusEq, "-=" },
    { TokenType::StarEq, "*=" },
    { TokenType::SlashEq, "/=" },
    { TokenType::Comma, "," },
    { TokenType::Semi, ";" },
    { TokenType::LParen, "(" },
    { TokenType::RParen, ")" },
    { TokenType::LBrace, "{" },
    { TokenType::RBrace, "}" },
    { TokenType::If, "if" },
    { TokenType::Else, "else" },
    { TokenType::While, "while" },
    { TokenType::Fn, "fn" },
    { TokenType::Return, "return" },
    { TokenType::Identifier, std::nullopt },
    { TokenType::String, "string" },
    { TokenType::FNumber, "f64" },
    { TokenType::INumber, "i32" },
    { TokenType::Bool, "bool" },
    { TokenType::True, "true" },
    { TokenType::False, "false" },
    { TokenType::Var, "var" },
    { TokenType::Const, "const" },
};

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
