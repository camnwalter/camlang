#pragma once

#include "token.hpp"
#include "type.hpp"

#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

struct AstNode {
    virtual ~AstNode() = default;

    virtual std::string print() = 0;
};

struct Expression : AstNode {
    Type type = Type::Unknown;
};

struct DoubleNode : Expression {
    double value;

    DoubleNode(double v) :
        value(v) {
        type = Type::Float64;
    }

    virtual std::string print() override {
        return std::format("DoubleNode({})", value);
    }
};

struct IntNode : Expression {
    uint32_t value;

    IntNode(uint32_t v) :
        value(v) {
        type = Type::Int32;
    }

    virtual std::string print() override {
        return std::format("IntNode({})", value);
    }
};

struct StringNode : Expression {
    std::string value;

    StringNode(std::string v) :
        value(std::move(v)) {
        type = Type::String;
    }

    virtual std::string print() override {
        return std::format("StringNode({})", value);
    }
};

struct IdentifierNode : Expression {
    std::string value;

    IdentifierNode(std::string v) :
        value(std::move(v)) {}

    virtual std::string print() override {
        return std::format("IdentifierNode({})", value);
    }
};

struct BoolNode : Expression {
    bool value;

    BoolNode(bool v) :
        value(v) {
        type = Type::Boolean;
    }

    virtual std::string print() override {
        return std::format("BoolNode({})", value);
    }
};

struct FunctionCall : Expression {
    std::unique_ptr<Expression> name;
    std::vector<std::unique_ptr<Expression>> args;

    FunctionCall(std::unique_ptr<Expression> n,
                 std::vector<std::unique_ptr<Expression>> a) :
        name(std::move(n)),
        args(std::move(a)) {}

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

struct UnaryOperator : Expression {
    Token tok;
    std::unique_ptr<Expression> child;

    UnaryOperator(Token t, std::unique_ptr<Expression> c) :
        tok(t),
        child(std::move(c)) {}

    virtual std::string print() override {
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
        tok(t),
        rhs(std::move(right)) {}

    virtual std::string print() override {
        return std::format("BinaryOperator(op='{}', lhs={}, rhs={})",
                           tok.lexeme,
                           lhs->print(),
                           rhs->print());
    }
};

struct Assignment : Expression {
    std::unique_ptr<Expression> lhs;
    Token tok;
    std::unique_ptr<Expression> rhs;

    Assignment(std::unique_ptr<Expression> left,
               Token t,
               std::unique_ptr<Expression> right) :
        lhs(std::move(left)),
        tok(t),
        rhs(std::move(right)) {
        type = rhs->type;
    }

    virtual std::string print() override {
        return std::format("Assignment(op='{}', lhs={}, rhs={})",
                           tok.lexeme,
                           lhs->print(),
                           rhs->print());
    }
};

struct Block : AstNode {
    std::vector<std::unique_ptr<AstNode>> nodes;

    virtual std::string print() override {
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

struct If : AstNode {
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Block> thenPart;
    std::unique_ptr<Block> elsePart;

    If(std::unique_ptr<Expression> cond,
       std::unique_ptr<Block> then,
       std::unique_ptr<Block> else_) :
        condition(std::move(cond)),
        thenPart(std::move(then)),
        elsePart(std::move(else_)) {}

    virtual std::string print() override {
        return std::format("If(cond={}, then={}, else={})",
                           condition->print(),
                           thenPart->print(),
                           elsePart->print());
    }
};

struct While : AstNode {
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Block> thenPart;

    While(std::unique_ptr<Expression> cond, std::unique_ptr<Block> then) :
        condition(std::move(cond)),
        thenPart(std::move(then)) {}

    virtual std::string print() override {
        return std::format("While(cond={}, thenPart={})",
                           condition->print(),
                           thenPart->print());
    }
};

struct ExpressionStatement : AstNode {
    std::unique_ptr<Expression> expression;

    ExpressionStatement(std::unique_ptr<Expression> expr) :
        expression(std::move(expr)) {}

    virtual std::string print() override {
        return std::format("ExpressionStatement(expr={})", expression->print());
    }
};

struct ReturnStatement : AstNode {
    std::unique_ptr<ExpressionStatement> expression;

    ReturnStatement(std::unique_ptr<ExpressionStatement> expr) :
        expression(std::move(expr)) {}

    virtual std::string print() override {
        return std::format("ReturnStatement(expression={})",
                           expression->print());
    }
};

struct EmptyStatement : AstNode {
    virtual std::string print() override {
        return "";
    }
};

struct Function : AstNode {
    std::unique_ptr<IdentifierNode> name;
    std::vector<std::unique_ptr<IdentifierNode>> params;
    std::unique_ptr<Block> block;

    Function(std::unique_ptr<IdentifierNode> n,
             std::vector<std::unique_ptr<IdentifierNode>> p,
             std::unique_ptr<Block> body) :
        name(std::move(n)),
        params(std::move(p)),
        block(std::move(body)) {}

    virtual std::string print() override {
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
    std::unique_ptr<ExpressionStatement> rhs; // nullable
    bool isConst;

    VarDeclaration(std::unique_ptr<IdentifierNode> n,
                   std::unique_ptr<ExpressionStatement> value,
                   bool const_) :
        name(std::move(n)),
        rhs(std::move(value)),
        isConst(const_) {}

    virtual std::string print() override {
        return std::format("VarDeclaration(type={}, name={}, value={})",
                           isConst ? "const" : "var",
                           name->print(),
                           rhs ? rhs->print() : "");
    }
};

struct File : AstNode {
    std::vector<std::unique_ptr<AstNode>> statements;

    virtual std::string print() override {
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
