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

    virtual std::ostream& print(std::ostream& os) const = 0;

    friend std::ostream& operator<<(std::ostream& os, const AstNode& node) {
        return node.print(os);
    }

    static size_t indent;
};

struct Expression : AstNode {
    const Type* type;
};

struct DoubleNode : Expression {
    double value;

    DoubleNode(double v) :
        value(v) {}

    std::ostream& print(std::ostream& os) const {
        os << "DoubleNode(" << value << ")";
        return os;
    }
};

struct IntNode : Expression {
    uint32_t value;

    IntNode(uint32_t v) :
        value(v) {}

    std::ostream& print(std::ostream& os) const {
        os << "IntNode(" << value << ")";
        return os;
    }
};

struct StringNode : Expression {
    std::string value;

    StringNode(std::string v) :
        value(std::move(v)) {}

    std::ostream& print(std::ostream& os) const {
        os << "StringNode(" << value << ")";
        return os;
    }
};

struct IdentifierNode : Expression {
    std::string_view value;

    IdentifierNode(std::string_view v) :
        value(std::move(v)) {}

    std::ostream& print(std::ostream& os) const {
        os << "IdentifierNode(" << value << ")";
        return os;
    }
};

struct BoolNode : Expression {
    bool value;

    BoolNode(bool v) :
        value(v) {}

    std::ostream& print(std::ostream& os) const {
        os << "BoolNode(" << value << ")";
        return os;
    }
};

struct FunctionCall : Expression {
    std::unique_ptr<Expression> name;
    std::vector<std::unique_ptr<Expression>> args {};

    FunctionCall(std::unique_ptr<Expression> n,
                 std::vector<std::unique_ptr<Expression>> a) :
        name(std::move(n)),
        args(std::move(a)) {}

    std::ostream& print(std::ostream& os) const {
        os << "FunctionCall(name: " << *name << ", args: (";
        auto sep = "";
        for (auto&& node : args) {
            os << *node << sep;
            sep = ", ";
        }
        os << "))" << std::endl;

        return os;
    }
};

struct UnaryOperator : Expression {
    Token tok;
    std::unique_ptr<Expression> child;

    UnaryOperator(Token t, std::unique_ptr<Expression> c) :
        tok(std::move(t)),
        child(std::move(c)) {}

    std::ostream& print(std::ostream& os) const {
        os << "UnaryOperator(op: " << tok.lexeme << ", child=" << *child << ")";
        return os;
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

    std::ostream& print(std::ostream& os) const {
        os
            << "UnaryOperator(op: "
            << tok.lexeme
            << ", lhs="
            << *lhs
            << ", rhs="
            << *rhs
            << ")";
        return os;
    }
};

struct Block : Expression {
    std::vector<std::unique_ptr<AstNode>> nodes;

    std::ostream& print(std::ostream& os) const {
        os << "Block(" << std::endl;
        indent += 2;
        for (auto&& node : nodes) {
            for (size_t i = 0; i < indent; i++) {
                os << " ";
            }

            os << *node << std::endl;
        }
        indent -= 2;

        for (size_t i = 0; i < indent; i++) {
            os << " ";
        }
        os << ")";

        return os;
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

    std::ostream& print(std::ostream& os) const {
        os
            << "IfExpression(condition: "
            << *condition
            << ", then: "
            << *thenPart
            << ", else: "
            << *elsePart
            << ")";
        return os;
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

    std::ostream& print(std::ostream& os) const {
        os
            << "Assignment(tok: "
            << tok.lexeme
            << ", lhs: "
            << *lhs
            << ", rhs: "
            << *rhs
            << ")";
        return os;
    }
};

struct While : AstNode {
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Block> thenPart;

    While(std::unique_ptr<Expression> cond, std::unique_ptr<Block> then) :
        condition(std::move(cond)),
        thenPart(std::move(then)) {}

    std::ostream& print(std::ostream& os) const {
        os
            << "While(cond: "
            << *condition
            << ", thenPart: "
            << *thenPart
            << ")";
        return os;
    }
};

struct ReturnStatement : AstNode {
    std::unique_ptr<Expression> expression;

    ReturnStatement(std::unique_ptr<Expression> expr) :
        expression(std::move(expr)) {}

    std::ostream& print(std::ostream& os) const {
        os << "ReturnStatement(expression: " << *expression << ")";
        return os;
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

    std::ostream& print(std::ostream& os) const {
        os
            << "IfExpression(condition: "
            << *condition
            << ", then: "
            << *thenPart
            << ", else: ";
        if (elsePart) {
            os << *elsePart;
        }
        os << ")";
        return os;
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

    std::ostream& print(std::ostream& os) const {
        os << "Function(name: " << *name << ", params: (";
        auto sep = "";
        for (auto&& node : params) {
            os << sep << *node;
            sep = ", ";
        }
        os << "), block: " << *block << ")" << std::endl;

        return os;
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

    std::ostream& print(std::ostream& os) const {
        os
            << "VarDeclaration("
            << (isConst ? "const" : "var")
            << ", name: "
            << *name
            << ", value: ";
        if (rhs) {
            os << *rhs;
        }
        os << ")";
        return os;
    }
};

struct File : AstNode {
    std::vector<std::unique_ptr<AstNode>> statements;

    std::ostream& print(std::ostream& os) const {
        os << "File(statements: (" << std::endl;

        indent += 2;
        for (auto&& node : statements) {
            for (size_t i = 0; i < indent; i++) {
                os << " ";
            }

            os << *node << std::endl;
        }
        indent -= 2;
        os << "))" << std::endl;
        return os;
    }

    void addStatement(std::unique_ptr<AstNode> node) {
        statements.push_back(std::move(node));
    }
};
