#pragma once

#include "token.hpp"

#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

struct AstNode {
    virtual ~AstNode() {}

    virtual std::string print() = 0;
};

struct DoubleNode : AstNode {
    double value;

    DoubleNode(double v) : value(v) {}

    virtual ~DoubleNode() override {}

    virtual std::string print() override {
        return std::format("DoubleNode({})", value);
    }
};

struct IntNode : AstNode {
    uint32_t value;

    IntNode(uint32_t v) : value(v) {}

    virtual ~IntNode() override {}

    virtual std::string print() override {
        return std::format("IntNode({})", value);
    }
};

struct StringNode : AstNode {
    std::string value;

    StringNode(std::string v) : value(v) {}

    virtual ~StringNode() override {}

    virtual std::string print() override {
        return std::format("StringNode({})", value);
    }
};

struct IdentifierNode : AstNode {
    std::string value;

    IdentifierNode(std::string v) : value(v) {}

    virtual ~IdentifierNode() override {}

    virtual std::string print() override {
        return std::format("IdentifierNode({})", value);
    }
};

struct BoolNode : AstNode {
    bool value;

    BoolNode(bool v) : value(v) {}

    virtual ~BoolNode() override {}

    virtual std::string print() override {
        return std::format("BoolNode({})", value);
    }
};

struct BinaryOperator : AstNode {
    AstNode* lhs;
    Token* tok;
    AstNode* rhs;

    BinaryOperator(AstNode* left, Token* t, AstNode* right) :
        lhs(left), tok(t), rhs(right) {}

    virtual ~BinaryOperator() override {}

    virtual std::string print() override {
        return std::format("BinaryOperator(op='{}', lhs={}, rhs={})",
                           tok->lexeme,
                           lhs->print(),
                           rhs->print());
    }
};

struct UnaryOperator : AstNode {
    Token* tok;
    AstNode* child;

    UnaryOperator(Token* t, AstNode* c) : tok(t), child(c) {}

    virtual ~UnaryOperator() override {}

    virtual std::string print() override {
        return std::format("UnaryOperator(op='{}', child={})",
                           tok->lexeme,
                           child->print());
    }
};

struct Block : AstNode {
    std::vector<AstNode*> nodes;

    virtual ~Block() override {}

    virtual std::string print() override {
        std::stringstream ss;
        ss << "Block(" << std::endl;
        auto sep = "";
        for (auto&& node : nodes) {
            ss << node->print() << sep;
            sep = "\n";
        }
        ss << ")";

        return ss.str();
    }

    void addNode(AstNode* node) {
        nodes.push_back(node);
    }
};

struct If : AstNode {
    AstNode* condition;
    Block* thenPart;
    Block* elsePart;

    If(AstNode* cond, Block* then, Block* else_) :
        condition(cond), thenPart(then), elsePart(else_) {}

    virtual ~If() override {}

    virtual std::string print() override {
        return std::format("If(cond={}, then={}, else={})",
                           condition->print(),
                           thenPart->print(),
                           elsePart->print());
    }
};

struct While : AstNode {
    AstNode* condition;
    AstNode* thenPart;

    While(AstNode* cond, AstNode* then) : condition(cond), thenPart(then) {}

    virtual ~While() override {}

    virtual std::string print() override {
        return std::format("While(cond={}, thenPart={})",
                           condition->print(),
                           thenPart->print());
    }
};

struct ExpressionStatement : AstNode {
    AstNode* expression;

    ExpressionStatement(AstNode* expr) : expression(expr) {}

    virtual ~ExpressionStatement() override {}

    virtual std::string print() override {
        return std::format("ExpressionStatement(expr={})", expression->print());
    }
};

struct ReturnStatement : AstNode {
    ExpressionStatement* expression;

    ReturnStatement(ExpressionStatement* expr) : expression(expr) {}

    virtual ~ReturnStatement() override {}

    virtual std::string print() override {
        return std::format("ReturnStatement(expression={})",
                           expression->print());
    }
};

struct Assignment : AstNode {
    AstNode* lhs;
    Token* tok;
    AstNode* rhs;

    Assignment(AstNode* left, Token* t, AstNode* right) :
        lhs(left), tok(t), rhs(right) {}

    virtual ~Assignment() override {}

    virtual std::string print() override {
        return std::format("Assignment(op='{}', lhs={}, rhs={})",
                           tok->lexeme,
                           lhs->print(),
                           rhs->print());
    }
};

struct Function : AstNode {
    IdentifierNode* name;
    std::vector<IdentifierNode*> params;
    Block* block;

    Function(IdentifierNode* n, std::vector<IdentifierNode*> p, Block* body) :
        name(n), params(p), block(body) {}

    virtual ~Function() override {}

    virtual std::string print() override {
        std::stringstream ss;
        ss << "Function(name=" << name->print() << ", params=(";
        auto sep = "";
        for (auto&& node : params) {
            ss << node->print() << sep;
            sep = "\n";
        }
        ss << ")";
        ss << ", block=" << block->print() << ")" << std::endl;

        return ss.str();
    }
};

struct FunctionCall : AstNode {
    AstNode* name;
    std::vector<AstNode*> args;

    FunctionCall(AstNode* n, std::vector<AstNode*> a) : name(n), args(a) {}

    virtual ~FunctionCall() override {}

    virtual std::string print() override {
        std::stringstream ss;
        ss << "FunctionCall(name=" << name->print() << ", args=(";
        auto sep = "";
        for (auto&& node : args) {
            ss << node->print() << sep;
            sep = "\n";
        }
        ss << "))" << std::endl;

        return ss.str();
    }
};

struct VarDeclaration : AstNode {
    IdentifierNode* name;
    ExpressionStatement* rhs; // nullable

    VarDeclaration(IdentifierNode* n, ExpressionStatement* value) :
        name(n), rhs(value) {}

    virtual ~VarDeclaration() override {}

    virtual std::string print() override {
        return std::format("VarDeclaration(name={}, value={})",
                           name->print(),
                           rhs ? rhs->print() : "");
    }
};

struct File : AstNode {
    std::vector<AstNode*> statements;

    virtual ~File() override {}

    virtual std::string print() override {
        std::stringstream ss;
        ss << "File(statements=(" << std::endl;

        auto sep = "";
        for (auto&& node : statements) {
            ss << node->print() << sep;
            sep = "\n";
        }
        ss << "))" << std::endl;
        return ss.str();
    }

    void addStatement(AstNode* node) {
        statements.push_back(node);
    }
};
