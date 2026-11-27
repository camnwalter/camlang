#pragma once

#include "token.hpp"

#include <memory>
#include <string>
#include <variant>
#include <vector>

struct ExpressionNode;
struct StatementNode;
struct TypeNode;
struct IdentifierNode;

using ExpressionPtr = std::unique_ptr<ExpressionNode>;
using StatementPtr = std::unique_ptr<StatementNode>;

// Metadata:

struct SourceLocation {
    size_t lineno;
    size_t colno;
};

SourceLocation tokenLocation(Token tok);

// Base nodes

struct IdentifierNode {
    std::string value;
};

struct StringLiteralNode {
    std::string value;
};

// Types

struct NamedTypeNode {
    IdentifierNode name;
};

struct ArrayTypeNode {
    std::unique_ptr<TypeNode> elementType;
};

struct TypeNode {
    SourceLocation loc;
    std::variant<NamedTypeNode, ArrayTypeNode> value;

    template <typename T>
    explicit TypeNode(SourceLocation location, T&& val) :
        loc(location),
        value(std::forward<T>(val)) {}
};

// Expressions

struct CallNode {
    ExpressionPtr name;
    std::vector<ExpressionPtr> args;
};

struct UnaryNode {
    TokenType op;
    ExpressionPtr child;
};

struct BinaryNode {
    TokenType op;
    ExpressionPtr lhs;
    ExpressionPtr rhs;
};

struct ExpressionNode {
    SourceLocation loc;
    std::variant<double,
                 uint32_t,
                 bool,
                 StringLiteralNode,
                 IdentifierNode,
                 CallNode,
                 UnaryNode,
                 BinaryNode>
        value;

    template <typename T>
    explicit ExpressionNode(SourceLocation location, T&& val) :
        loc(location),
        value(std::forward<T>(val)) {}
};

// Statements

struct ExpressionStatementNode {
    ExpressionPtr expr;

    explicit ExpressionStatementNode(ExpressionPtr e) :
        expr(std::move(e)) {}
};

struct BlockNode {
    std::vector<StatementPtr> nodes;
};

struct AssignmentNode {
    TokenType op;
    ExpressionPtr lhs;
    ExpressionPtr rhs;
};

struct WhileNode {
    ExpressionPtr condition;
    BlockNode body;
};

struct ReturnNode {
    ExpressionPtr expression;
};

struct IfNode {
    using ElseClause = std::variant<std::monostate, StatementPtr, BlockNode>;

    ExpressionPtr condition;
    BlockNode thenPart;
    ElseClause elsePart;
};

struct VarDeclNode {
    IdentifierNode name;
    std::variant<std::monostate, TypeNode> type;
    std::variant<std::monostate, ExpressionPtr> rhs;
    bool isConst;
};

struct ParamNode {
    IdentifierNode name;
    TypeNode type;
};

struct FunctionNode {
    IdentifierNode name;
    std::vector<ParamNode> params;
    TypeNode returnType;
    BlockNode body;
};

struct StatementNode {
    SourceLocation loc;
    std::variant<ExpressionStatementNode,
                 BlockNode,
                 AssignmentNode,
                 WhileNode,
                 ReturnNode,
                 IfNode,
                 VarDeclNode,
                 FunctionNode>
        value;

    template <typename T>
    explicit StatementNode(SourceLocation location, T&& val) :
        loc(location),
        value(std::forward<T>(val)) {}
};

struct FileNode {
    std::vector<StatementPtr> statements;
};
