#pragma once

#include <vector>

#include "token.hpp"

template <typename Base, typename T>
inline bool instanceof(const T *ptr)
{
    return dynamic_cast<const Base *>(ptr) != nullptr;
}

struct AstNode
{
    virtual ~AstNode() {}

    virtual void print() = 0;
};

struct IntNode : AstNode
{
    int32_t value;

    IntNode(int32_t v) : value(v) {}

    virtual ~IntNode() override {}

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

struct BinaryOperator : AstNode
{
    TokenType tt;
    AstNode *lhs;
    AstNode *rhs;

    BinaryOperator(AstNode *left, Token *tok, AstNode *right) : tt(tok->tt), lhs(left), rhs(right)
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

class Parser
{
private:
    std::vector<Token *> tokens;
    uint32_t index;

public:
    Parser(std::vector<Token *> toks) : tokens(toks)
    {
        index = 0;
    }

    bool isAtEnd()
    {
        return peek()->tt == eof;
    }

    Token *peek()
    {
        return tokens[index];
    }

    // gets current and moves to next
    Token *advance()
    {
        if (!isAtEnd())
        {
            next();
        }
        return getPrev();
    }

    Token *getPrev()
    {
        return tokens[index - 1];
    }

    void next()
    {
        index++;
    }

    // if current token has type type, returns true. else returns false
    bool check(TokenType type)
    {
        return peek()->tt == type;
    }

    // if current token has type type, returns true and advances. else returns false
    bool match(TokenType type)
    {
        if (check(type))
        {
            next();
            return true;
        }
        return false;
    }

    [[noreturn]] void fail(const char *message)
    {
        throw message;
    }

    Token *expect(TokenType type, const char *message)
    {
        if (check(type))
        {
            return advance();
        }

        fail(message);
    }

    AstNode *primary()
    {
        if (check(STRING))
        {
            return new StringNode(std::get<std::string>(advance()->value));
        }
        if (check(NUMBER))
        {
            return new IntNode(std::get<int32_t>(advance()->value));
        }

        fail("Invalid primary expr");
    }

    AstNode *unary()
    {
        return primary();
    }

    AstNode *factor()
    {
        AstNode *expr = unary();
        while (match(TIMES) || match(DIVIDE))
        {
            Token *op = getPrev();
            AstNode *rhs = unary();
            expr = new BinaryOperator(expr, op, rhs);
        }

        return expr;
    }

    AstNode *term()
    {
        AstNode *expr = factor();
        while (match(PLUS) || match(MINUS))
        {
            Token *op = getPrev();
            AstNode *rhs = factor();
            expr = new BinaryOperator(expr, op, rhs);
        }

        return expr;
    }

    AstNode *comparison()
    {
        return term();
    }

    AstNode *equality()
    {
        return comparison();
    }

    AstNode *expression()
    {
        return equality();
    }

    AstNode *parse()
    {
        return expression();
    }
};
