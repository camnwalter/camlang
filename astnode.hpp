#pragma once

#include <memory>
#include <iostream>
#include <vector>

#include "token.hpp"

struct AstNode
{
    virtual ~AstNode() {}

    virtual void print() = 0;
};

using AstNodePtr = std::shared_ptr<AstNode>;

struct DoubleNode : AstNode
{
    double value;

    DoubleNode(double v) : value(v) {}

    virtual ~DoubleNode() override {}

    virtual void print() override
    {
        std::cout << value;
    }
};

struct StringNode : AstNode
{
    std::string value;

    StringNode(std::string v) : value(v) {}

    virtual ~StringNode() override {}

    virtual void print() override
    {
        std::cout << value;
    }
};

struct IdentifierNode : AstNode
{
    std::string value;

    IdentifierNode(std::string v) : value(v) {}

    virtual ~IdentifierNode() override {}

    virtual void print() override
    {
        std::cout << value;
    }
};

struct BoolNode : AstNode
{
    bool value;

    BoolNode(bool v) : value(v) {}

    virtual ~BoolNode() override {}

    virtual void print() override
    {
        std::cout << value;
    }
};

struct BinaryOperator : AstNode
{
    TokenType tt;
    AstNodePtr lhs;
    AstNodePtr rhs;

    BinaryOperator(AstNodePtr left, TokenPtr tok, AstNodePtr right) : tt(tok->tt), lhs(left), rhs(right)
    {
    }

    virtual ~BinaryOperator() override {}

    virtual void print() override
    {
        std::cout << "BinaryOperator(lhs=";
        lhs->print();
        std::cout << ", tt=" << tt << ", rhs=";
        rhs->print();
        std::cout << ")";
    }
};

struct UnaryOperator : AstNode
{
    TokenType tt;
    AstNodePtr child;

    UnaryOperator(TokenPtr tok, AstNodePtr c) : tt(tok->tt), child(c)
    {
    }

    virtual ~UnaryOperator() override {}

    virtual void print() override
    {
        std::cout << "UnaryOperator(tt=" << tt << ", child=";
        child->print();
        std::cout << ")";
    }
};

struct Block : AstNode
{
    std::vector<AstNodePtr> nodes;

    virtual ~Block() override {}

    virtual void print() override
    {
        std::cout << "Block(";
        for (auto &&node : nodes)
        {
            node->print();
        }
        std::cout << ")" << std::endl;
    }

    void addNode(AstNodePtr node)
    {
        if (auto block = std::dynamic_pointer_cast<Block>(node))
        {
            nodes.insert(nodes.end(), block->nodes.begin(), block->nodes.end());
        }
        else
        {
            nodes.push_back(node);
        }
    }
};
