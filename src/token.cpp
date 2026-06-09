#include "../include/token.hpp"

std::unordered_map<std::string, TokenType> reservedKeywords = {
    {"mod", TokenType::Mod},
    {"xor", TokenType::Xor},
    {"and", TokenType::And},
    {"or", TokenType::Or},
    {"if", TokenType::If},
    {"else", TokenType::Else},
    {"while", TokenType::While},
    {"fn", TokenType::Fn},
    {"return", TokenType::Return},
    {"string", TokenType::String},
    {"f64", TokenType::FNumber},
    {"i32", TokenType::INumber},
    {"bool", TokenType::Bool},
    {"true", TokenType::True},
    {"false", TokenType::False},
    {"var", TokenType::Var},
    {"const", TokenType::Const},
};
