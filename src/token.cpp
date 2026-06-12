#include "../include/token.hpp"

const std::unordered_map<std::string_view, TokenType> reservedKeywords = {
    {"mod", TokenType::Mod},
    {"xor", TokenType::Xor},
    {"and", TokenType::And},
    {"or", TokenType::Or},
    {"if", TokenType::If},
    {"else", TokenType::Else},
    {"while", TokenType::While},
    {"fn", TokenType::Fn},
    {"return", TokenType::Return},
    {"true", TokenType::True},
    {"false", TokenType::False},
    {"var", TokenType::Var},
    {"const", TokenType::Const},
    // Type names are just normal identifiers, they don't need special tokens
};
