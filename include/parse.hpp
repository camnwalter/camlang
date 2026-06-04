#pragma once

#include "astnode.hpp"
#include "exceptions.hpp"
#include "token.hpp"

#include <vector>

class Parser {
private:
    std::vector<Token*> tokens;
    uint32_t index;

public:
    Parser(std::vector<Token*> toks) : tokens(toks), index(0) {}

    bool isAtEnd() {
        return peek()->tt == TokenType::Eof;
    }

    Token* peek() {
        return tokens[index];
    }

    // gets current and moves to next
    Token* advance() {
        if (!isAtEnd()) {
            next();
        }
        return prev();
    }

    Token* prev() {
        return tokens[index - 1];
    }

    void next() {
        index++;
    }

    void before() {
        index--;
    }

    // if current token has type type, returns true. else returns false
    bool check(std::same_as<TokenType> auto... types) {
        for (TokenType t : { types... }) {
            if (peek()->tt == t) {
                return true;
            }
        }

        return false;
    }

    // checks and advances if current type matches
    bool match(std::same_as<TokenType> auto... types) {
        if (check(types...)) {
            next();
            return true;
        }

        return false;
    }

    Token* expect(TokenType type, const char* message) {
        if (check(type)) {
            return advance();
        }

        SyntaxError(message, peek()->lineno, peek()->colno);
    }

    IdentifierNode* identifier();
    std::vector<AstNode*> args();
    AstNode* primary();
    AstNode* call();
    AstNode* exponent();
    AstNode* unary();
    AstNode* factor();
    AstNode* term();
    AstNode* shifts();
    AstNode* comparison();
    AstNode* equality();
    AstNode* bitandexpr();
    AstNode* xorexpr();
    AstNode* bitorexpr();
    AstNode* andexpr();
    AstNode* orexpr();
    AstNode* expression();
    AstNode* declaration();
    ExpressionStatement* expressionStatement();
    Block* block();
    AstNode* statement();
    AstNode* file();

    AstNode* parse() {
        std::cout << "Parsing..." << std::endl;
        return file();
    }
};
