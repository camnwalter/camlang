#pragma once

#include "astnode.hpp"

#include <unordered_map>
#include <variant>

struct Printer {
    size_t indent {0};
    std::unordered_map<TokenType, std::string> typeToString = {
        {TokenType::Eof, "Eof"},
        {TokenType::Error, "Error"},
        {TokenType::Plus, "Plus"},
        {TokenType::Minus, "Minus"},
        {TokenType::Star, "Star"},
        {TokenType::Slash, "Slash"},
        {TokenType::Caret, "Caret"},
        {TokenType::Mod, "Mod"},
        {TokenType::Bang, "Bang"},
        {TokenType::Less, "Less"},
        {TokenType::LessEq, "LessEq"},
        {TokenType::Greater, "Greater"},
        {TokenType::GreaterEq, "GreaterEq"},
        {TokenType::EqEq, "EqEq"},
        {TokenType::BangEq, "BangEq"},
        {TokenType::LShift, "LShift"},
        {TokenType::RShift, "RShift"},
        {TokenType::Tilde, "Tilde"},
        {TokenType::BitAnd, "BitAnd"},
        {TokenType::Xor, "Xor"},
        {TokenType::BitOr, "BitOr"},
        {TokenType::And, "And"},
        {TokenType::Or, "Or"},
        {TokenType::Eq, "Eq"},
        {TokenType::PlusEq, "PlusEq"},
        {TokenType::MinusEq, "MinusEq"},
        {TokenType::StarEq, "StarEq"},
        {TokenType::SlashEq, "SlashEq"},
        {TokenType::Comma, "Comma"},
        {TokenType::Semi, "Semi"},
        {TokenType::Colon, "Colon"},
        {TokenType::LParen, "LParen"},
        {TokenType::RParen, "RParen"},
        {TokenType::LBrace, "LBrace"},
        {TokenType::RBrace, "RBrace"},
        {TokenType::If, "If"},
        {TokenType::Else, "Else"},
        {TokenType::While, "While"},
        {TokenType::Return, "Return"},
        {TokenType::Identifier, "Identifier"},
        {TokenType::String, "String"},
        {TokenType::FNumber, "FNumber"},
        {TokenType::INumber, "INumber"},
        {TokenType::True, "True"},
        {TokenType::False, "False"},
        {TokenType::Var, "Var"},
        {TokenType::Const, "Const"},
        {TokenType::Fn, "Fn"},
    };

    void operator()(const std::monostate&) {
        std::cout << "nil";
    }

    void operator()(const ExpressionPtr& node) {
        std::visit(*this, node->value);
    }

    void operator()(const StatementPtr& node) {
        std::visit(*this, node->value);
    }

    void operator()(const TypePtr& node) {
        std::visit(*this, node->value);
    }

    void operator()(double node) {
        std::cout << node;
    }

    void operator()(uint32_t node) {
        std::cout << node;
    }

    void operator()(bool node) {
        std::cout << node;
    }

    void operator()(const StringLiteralNode& node) {
        std::cout << "\"" << node.value << "\"";
    }

    void operator()(const IdentifierNode& node) {
        std::cout << "Identifier(" << node.value << ")";
    }

    void operator()(const CallNode& node) {
        std::cout << "Call(";
        std::visit(*this, node.name->value);
        std::cout << ", args: (";

        auto sep = "";
        for (auto&& arg : node.args) {
            std::cout << sep;
            std::visit(*this, arg->value);
            sep = ", ";
        }

        std::cout << "))";
    }

    void operator()(const UnaryNode& node) {
        std::cout << "Unary(op: " << typeToString[node.op] << ", child: ";
        std::visit(*this, node.child->value);
        std::cout << ")";
    }

    void operator()(const BinaryNode& node) {
        std::cout << "Binary(op: " << typeToString[node.op] << ", lhs: ";
        std::visit(*this, node.lhs->value);
        std::cout << ", rhs: ";
        std::visit(*this, node.rhs->value);
        std::cout << ")";
    }

    // Statements

    void operator()(const ExpressionStatementNode& node) {
        std::visit(*this, node.expr->value);
    }

    void operator()(const BlockNode& node) {
        std::cout << "Block(";
        indent += 2;
        for (auto&& statement : node.nodes) {
            std::cout << std::endl;

            for (size_t i = 0; i < indent; i++) {
                std::cout << " ";
            }

            std::visit(*this, statement->value);
        }

        std::cout << std::endl;
        indent -= 2;
        for (size_t i = 0; i < indent; i++) {
            std::cout << " ";
        }
        std::cout << ")";
    }

    void operator()(const AssignmentNode& node) {
        std::cout << "Assign(op: " << typeToString[node.op] << ", lhs: ";
        std::visit(*this, node.lhs->value);
        std::cout << ", rhs: ";
        std::visit(*this, node.rhs->value);
        std::cout << ")";
    }

    void operator()(const WhileNode& node) {
        std::cout << "While(condition: ";
        std::visit(*this, node.condition->value);
        std::cout << ", body: ";
        (*this)(node.body);
        std::cout << ")";
    }

    void operator()(const ReturnNode& node) {
        std::cout << "Return(expression: ";
        std::visit(*this, node.expression->value);
        std::cout << ")";
    }

    void operator()(const IfNode& node) {
        std::cout << "If(condition: ";
        std::visit(*this, node.condition->value);
        std::cout << ", then: ";
        (*this)(node.thenPart);
        std::cout << ", else: ";
        std::visit(*this, node.elsePart);
        std::cout << ")";
    }

    void operator()(const VarDeclNode& node) {
        std::cout << "VarDeclaration(type: ";
        std::cout << (node.isConst ? "const" : "var");
        std::cout << ", name: ";
        (*this)(node.name);
        std::cout << ", type: ";
        std::visit(*this, node.type);
        std::cout << ", value: ";
        std::visit(*this, node.rhs);
        std::cout << ")";
    }

    void operator()(const FunctionNode& node) {
        std::cout << "Function(name: ";
        (*this)(node.name);
        std::cout << ", params: (";

        auto sep = "";
        for (auto&& param : node.params) {
            std::cout << sep;
            (*this)(param.name);
            std::cout << ": ";
            std::visit(*this, param.type->value);
            sep = ", ";
        }
        std::cout << "), returnType: ";
        (*this)(node.returnType);
        std::cout << ", body: ";
        (*this)(node.body);
        std::cout << ")";
    }

    void operator()(const FileNode& node) {
        for (auto&& declaration : node.statements) {
            std::visit(*this, declaration->value);
            std::cout << std::endl;
        }
    }

    // Types

    void operator()(const NamedTypeNode& node) {
        (*this)(node.name);
    }

    void operator()(const ArrayTypeNode& node) {
        std::visit(*this, node.elementType->value);
        std::cout << "[]";
    }
};
