#pragma once

#include "token.hpp"

#include <memory>
#include <string>
#include <variant>
#include <vector>

struct Expression;
struct Statement;

using ExpressionPtr = std::unique_ptr<Expression>;
using StatementPtr = std::unique_ptr<Statement>;

// Expressions

struct String {
    std::string value;
};

struct Identifier {
    std::string_view value;
};

struct Call {
    ExpressionPtr name;
    std::vector<ExpressionPtr> args;
};

struct Unary {
    TokenType op;
    ExpressionPtr child;
};

struct Binary {
    TokenType op;
    ExpressionPtr lhs;
    ExpressionPtr rhs;
};

struct Expression {
    std::variant<double,
                 uint32_t,
                 bool,
                 String,
                 Identifier,
                 Call,
                 Unary,
                 Binary>
        value;

    template <typename T>
    Expression(T&& val) :
        value(std::forward<T>(val)) {}
};

// Statements

struct ExpressionStatement {
    ExpressionPtr expr;

    explicit ExpressionStatement(ExpressionPtr e) :
        expr(std::move(e)) {}
};

struct Block {
    std::vector<StatementPtr> nodes;
};

struct Assign {
    TokenType op;
    ExpressionPtr lhs;
    ExpressionPtr rhs;
};

struct While {
    ExpressionPtr condition;
    Block body;
};

struct Return {
    ExpressionPtr expression;
};

struct If {
    using ElseClause = std::variant<std::monostate, StatementPtr, Block>;

    ExpressionPtr condition;
    Block thenPart;
    ElseClause elsePart;
};

struct VarDeclaration {
    Identifier name;
    std::variant<std::monostate, Identifier> type;
    std::variant<std::monostate, ExpressionPtr> rhs;
    bool isConst;
};

struct Function {
    Identifier name;
    std::vector<std::pair<Identifier, Identifier>> params;
    Identifier returnType;
    Block body;
};

struct Statement {
    std::variant<ExpressionStatement,
                 Block,
                 Assign,
                 While,
                 Return,
                 If,
                 VarDeclaration,
                 Function>
        value;

    template <typename T>
    Statement(T&& v) :
        value(std::forward<T>(v)) {}
};

struct File {
    std::vector<StatementPtr> statements;
};
