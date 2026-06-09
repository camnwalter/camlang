#pragma once

#include "astnode.hpp"
#include "exceptions.hpp"
#include "symbol.hpp"
#include "token.hpp"

#include <vector>

class Parser {
private:
    std::vector<Token> tokens;
    uint32_t index;
    SymbolTable* const symbolTable;

public:
    Parser(SymbolTable* const symTab, std::vector<Token>&& toks) :
        tokens(std::move(toks)),
        index(0),
        symbolTable(symTab) {}

    bool isAtEnd() const {
        return peek().tokenType == TokenType::Eof;
    }

    const Token& peek() const {
        return tokens[index];
    }

    // gets current and moves to next
    const Token& advance() {
        if (!isAtEnd()) {
            next();
        }
        return prev();
    }

    const Token& prev() const {
        return tokens[index - 1];
    }

    void next() {
        index++;
    }

    void before() {
        index--;
    }

    // if current token has type type, returns true. else returns false
    bool check(std::same_as<TokenType> auto... types) const {
        for (TokenType t : {types...}) {
            if (peek().tokenType == t) {
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

    const Token& expect(TokenType type, const char* message) {
        if (check(type)) {
            return advance();
        }

        SyntaxError(message, peek().line, peek().column);
    }

    std::unique_ptr<IdentifierNode> identifier();
    std::vector<std::unique_ptr<Expression>> args();
    std::unique_ptr<Expression> primary();
    std::unique_ptr<Expression> call();
    std::unique_ptr<Expression> unary();
    std::unique_ptr<Expression> factor();
    std::unique_ptr<Expression> term();
    std::unique_ptr<Expression> shifts();
    std::unique_ptr<Expression> comparison();
    std::unique_ptr<Expression> equality();
    std::unique_ptr<Expression> bitandexpr();
    std::unique_ptr<Expression> xorexpr();
    std::unique_ptr<Expression> bitorexpr();
    std::unique_ptr<Expression> andexpr();
    std::unique_ptr<Expression> orexpr();
    std::unique_ptr<Expression> expression();
    std::unique_ptr<AstNode> declaration();
    std::unique_ptr<ExpressionStatement> expressionStatement();
    std::unique_ptr<Block>
        block(std::unordered_map<std::string, Symbol>&& symbolsToAdd);
    std::unique_ptr<AstNode> statement();
    std::unique_ptr<File> file();

    std::unique_ptr<File> parse() {
        std::cout << "Parsing..." << std::endl;
        return file();
    }
};
