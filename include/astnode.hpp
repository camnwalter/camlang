#pragma once

#include "token.hpp"
#include "type.hpp"

#include <cstdint>
#include <format>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

struct AstNode {
    AstNode() = default;

    AstNode(const AstNode&) = delete;
    AstNode(AstNode&&) = delete;

    void operator=(const AstNode&) = delete;
    AstNode& operator=(AstNode&&) = delete;

    virtual ~AstNode() = default;

    virtual std::string print() = 0;
};

struct Expression : AstNode {
    const Type* type;
};

struct DoubleNode : Expression {
    double value;

    DoubleNode(double v) :
        value(v) {}

    std::string print() override {
        return std::format("DoubleNode({})", value);
    }
};

struct IntNode : Expression {
    uint32_t value;

    IntNode(uint32_t v) :
        value(v) {}

    std::string print() override {
        return std::format("IntNode({})", value);
    }
};

struct StringNode : Expression {
    std::string value;

    StringNode(std::string v) :
        value(std::move(v)) {}

    std::string print() override {
        return std::format("StringNode({})", value);
    }
};

struct IdentifierNode : Expression {
    std::string_view value;

    IdentifierNode(std::string_view v) :
        value(std::move(v)) {}

    std::string print() override {
        return std::format("IdentifierNode({})", value);
    }
};

struct BoolNode : Expression {
    bool value;

    BoolNode(bool v) :
        value(v) {}

    std::string print() override {
        return std::format("BoolNode({})", value);
    }
};

struct FunctionCall : Expression {
    std::unique_ptr<Expression> name;
    std::vector<std::unique_ptr<Expression>> args {};

    FunctionCall(std::unique_ptr<Expression> n,
                 std::vector<std::unique_ptr<Expression>> a) :
        name(std::move(n)),
        args(std::move(a)) {}

    std::string print() override {
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

struct UnaryOperator : Expression {
    Token tok;
    std::unique_ptr<Expression> child;

    UnaryOperator(Token t, std::unique_ptr<Expression> c) :
        tok(std::move(t)),
        child(std::move(c)) {}

    std::string print() override {
        return std::format("UnaryOperator(op='{}', child={})",
                           tok.lexeme,
                           child->print());
    }
};

struct BinaryOperator : Expression {
    std::unique_ptr<Expression> lhs;
    Token tok;
    std::unique_ptr<Expression> rhs;

    BinaryOperator(std::unique_ptr<Expression> left,
                   Token t,
                   std::unique_ptr<Expression> right) :
        lhs(std::move(left)),
        tok(std::move(t)),
        rhs(std::move(right)) {}

    std::string print() override {
        return std::format("BinaryOperator(op='{}', lhs={}, rhs={})",
                           tok.lexeme,
                           lhs->print(),
                           rhs->print());
    }
};

struct Block : Expression {
    std::vector<std::unique_ptr<AstNode>> nodes;

    std::string print() override {
        std::stringstream ss;
        ss << "Block(" << std::endl;
        for (auto&& node : nodes) {
            ss << node->print() << std::endl;
        }
        ss << ")";

        return ss.str();
    }

    void addNode(std::unique_ptr<AstNode> node) {
        nodes.push_back(std::move(node));
    }
};

struct IfExpression : Expression {
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Block> thenPart;
    // either Block or IfExpression
    std::unique_ptr<Expression> elsePart;

    IfExpression(std::unique_ptr<Expression> cond,
                 std::unique_ptr<Block> then,
                 std::unique_ptr<Expression> else_) :
        condition(std::move(cond)),
        thenPart(std::move(then)),
        elsePart(std::move(else_)) {}

    std::string print() override {
        return std::format("IfExpression(cond={}, then={}, else={})",
                           condition->print(),
                           thenPart->print(),
                           elsePart->print());
    }
};

struct Assignment : AstNode {
    std::unique_ptr<Expression> lhs;
    Token tok;
    std::unique_ptr<Expression> rhs;

    Assignment(std::unique_ptr<Expression> left,
               Token t,
               std::unique_ptr<Expression> right) :
        lhs(std::move(left)),
        tok(std::move(t)),
        rhs(std::move(right)) {}

    std::string print() override {
        return std::format("Assignment(op='{}', lhs={}, rhs={})",
                           tok.lexeme,
                           lhs->print(),
                           rhs->print());
    }
};

struct While : AstNode {
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Block> thenPart;

    While(std::unique_ptr<Expression> cond, std::unique_ptr<Block> then) :
        condition(std::move(cond)),
        thenPart(std::move(then)) {}

    std::string print() override {
        return std::format("While(cond={}, thenPart={})",
                           condition->print(),
                           thenPart->print());
    }
};

struct ReturnStatement : AstNode {
    std::unique_ptr<Expression> expression;

    ReturnStatement(std::unique_ptr<Expression> expr) :
        expression(std::move(expr)) {}

    std::string print() override {
        return std::format("ReturnStatement(expression={})",
                           expression->print());
    }
};

struct IfStatement : Expression {
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Block> thenPart;
    // either null or Block or IfStatement
    std::unique_ptr<AstNode> elsePart;

    IfStatement(std::unique_ptr<Expression> cond,
                std::unique_ptr<Block> then,
                std::unique_ptr<AstNode> else_) :
        condition(std::move(cond)),
        thenPart(std::move(then)),
        elsePart(std::move(else_)) {}

    std::string print() override {
        return std::format("IfStatement(cond={}, then={}, else={})",
                           condition->print(),
                           thenPart->print(),
                           elsePart ? elsePart->print() : "");
    }
};

struct Function : AstNode {
    std::unique_ptr<IdentifierNode> name;
    std::vector<std::unique_ptr<IdentifierNode>> params;
    std::unique_ptr<IdentifierNode> returnType;
    std::unique_ptr<Block> block;

    Function(std::unique_ptr<IdentifierNode> n,
             std::vector<std::unique_ptr<IdentifierNode>> p,
             std::unique_ptr<IdentifierNode> ret,
             std::unique_ptr<Block> body) :
        name(std::move(n)),
        params(std::move(p)),
        returnType(std::move(ret)),
        block(std::move(body)) {}

    std::string print() override {
        std::stringstream ss;
        ss << "Function(name=" << name->print() << ", params=(";
        auto sep = "";
        for (auto&& node : params) {
            ss << sep << node->print();
            sep = ", ";
        }
        ss << ")";
        ss << ", block=" << block->print() << ")" << std::endl;

        return ss.str();
    }
};

struct VarDeclaration : AstNode {
    std::unique_ptr<IdentifierNode> name;
    std::unique_ptr<Expression> rhs; // nullable
    bool isConst;

    VarDeclaration(std::unique_ptr<IdentifierNode> n,
                   std::unique_ptr<Expression> value,
                   bool const_) :
        name(std::move(n)),
        rhs(std::move(value)),
        isConst(const_) {}

    std::string print() override {
        return std::format("VarDeclaration(type={}, name={}, value={})",
                           isConst ? "const" : "var",
                           name->print(),
                           rhs ? rhs->print() : "");
    }
};

struct File : AstNode {
    std::vector<std::unique_ptr<AstNode>> statements;

    std::string print() override {
        std::stringstream ss;
        ss << "File(statements=(" << std::endl;

        for (auto&& node : statements) {
            ss << node->print() << std::endl;
        }
        ss << "))" << std::endl;
        return ss.str();
    }

    void addStatement(std::unique_ptr<AstNode> node) {
        statements.push_back(std::move(node));
    }
};
