#pragma once

#include <vector>
#include <memory>

#include "token.hpp"
#include "astnode.hpp"

class Parser
{
private:
    std::vector<TokenPtr> tokens;
    uint32_t index;

public:
    Parser(std::vector<TokenPtr> toks) : tokens(toks), index(0)
    {
    }

    bool isAtEnd()
    {
        return peek()->tt == eof;
    }

    TokenPtr peek()
    {
        return tokens[index];
    }

    // gets current and moves to next
    TokenPtr advance()
    {
        if (!isAtEnd())
        {
            next();
        }
        return prev();
    }

    TokenPtr prev()
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
    bool match(std::same_as<TokenType> auto... types)
    {
        for (TokenType t : {types...})
        {
            if (check(t))
            {
                next();
                return true;
            }
        }

        return false;
    }

    TokenPtr expect(TokenType type, const char *message)
    {
        if (check(type))
        {
            return advance();
        }

        throw SyntaxError(message, peek()->lineno, peek()->colno);
    }

    AstNodePtr primary()
    {
        if (match(STRING))
        {
            return std::make_shared<StringNode>(std::get<std::string>(prev()->value));
        }
        if (match(NUMBER))
        {
            return std::make_shared<DoubleNode>(std::get<double>(prev()->value));
        }
        if (match(IDENTIFIER))
        {
            return std::make_shared<IdentifierNode>(std::get<std::string>(prev()->value));
        }
        if (match(TRUE))
        {
            return std::make_shared<BoolNode>(true);
        }
        if (match(FALSE))
        {
            return std::make_shared<BoolNode>(false);
        }
        if (match(LPAREN))
        {
            AstNodePtr node = expression();
            expect(RPAREN, "expected closing )");
            return node;
        }

        throw SyntaxError("Invalid primary expr", peek()->lineno, peek()->colno);
    }

    AstNodePtr exponent()
    {
        AstNodePtr node = primary();
        while (match(CARET))
        {
            TokenPtr op = prev();
            node = std::make_shared<BinaryOperator>(node, op, exponent());
        }
        return node;
    }

    AstNodePtr unary()
    {
        if (match(MINUS, PLUS))
        {
            return std::make_shared<UnaryOperator>(prev(), exponent());
        }

        if (match(NOT))
        {
            return std::make_shared<UnaryOperator>(prev(), unary());
        }

        return exponent();
    }

    AstNodePtr factor()
    {
        AstNodePtr expr = unary();
        while (match(STAR, SLASH))
        {
            TokenPtr op = prev();
            expr = std::make_shared<BinaryOperator>(expr, op, unary());
        }

        return expr;
    }

    AstNodePtr term()
    {
        AstNodePtr expr = factor();
        while (match(PLUS, MINUS))
        {
            TokenPtr op = prev();
            expr = std::make_shared<BinaryOperator>(expr, op, factor());
        }

        return expr;
    }

    AstNodePtr comparison()
    {
        AstNodePtr node = term();
        while (match(LT, LE, GT, GE))
        {
            node = std::make_shared<BinaryOperator>(node, prev(), term());
        }
        return node;
    }

    AstNodePtr equality()
    {
        AstNodePtr node = comparison();
        while (match(EQ, NE))
        {
            node = std::make_shared<BinaryOperator>(node, prev(), comparison());
        }
        return node;
    }

    AstNodePtr expression()
    {
        return equality();
    }

    AstNodePtr statement()
    {
        if (match(LBRACE))
        {
            std::shared_ptr<Block> block = std::make_shared<Block>();
            while (!check(RBRACE))
            {
                block->addNode(statement());
            }
            expect(RBRACE, "expected }");
            return block;
        }

        AstNodePtr expr = expression();
        expect(SEMICOLON, "; expected");
        return expr;
    }

    AstNodePtr block()
    {
        expect(LBRACE, "{ expected");

        auto block = std::make_shared<Block>();
        while (!check(RBRACE))
        {
            block->addNode(statement());
        }
        expect(RBRACE, "expected }");
        return block;
    }

    AstNodePtr parse()
    {
        std::cout << "Parsing..." << std::endl;
        return block();
    }
};
