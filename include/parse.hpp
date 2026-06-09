#pragma once

#include "astnode.hpp"
#include "exceptions.hpp"
#include "symbol.hpp"
#include "token.hpp"

#include <functional>
#include <vector>

enum class Precedence {
    Lowest,
    Assignment,
    Or,
    And,
    BitOr,
    Xor,
    BitAnd,
    Equality,
    Comparison,
    Shift,
    Plus,
    Times,
    Unary,
    Call,
    Primary,
};

extern std::unordered_map<TokenType, Precedence> precedenceMap;

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

    Token peek() const {
        return tokens[index];
    }

    // gets current and moves to next
    Token advance() {
        if (!isAtEnd()) {
            next();
        }
        return prev();
    }

    Token prev() const {
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

    Token expect(TokenType type, const char* message) {
        if (check(type)) {
            return advance();
        }

        SyntaxError(message, peek().line, peek().column);
    }

    Precedence nextPrecedence() {
        auto type = peek().tokenType;
        if (precedenceMap.contains(type)) {
            return precedenceMap[type];
        }

        // TODO: Verify this is what we actually want.
        return Precedence::Lowest;
    }

    std::unique_ptr<Expression> parseInfix(std::unique_ptr<Expression>);

    std::unique_ptr<FunctionCall>
        parseFunctionCall(Token, std::unique_ptr<Expression>);
    std::unique_ptr<Assignment> parseAssignment(Token,
                                                std::unique_ptr<Expression>);
    std::unique_ptr<Expression> parseExpression(Precedence);

    std::unique_ptr<IdentifierNode> identifier();
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

using UnaryParselet =
    std::function<std::unique_ptr<Expression>(Parser*, Token)>;
using BinaryParselet = std::function<
    std::unique_ptr<Expression>(Parser*, Token, std::unique_ptr<Expression>)>;
extern std::unordered_map<TokenType, UnaryParselet> primaryAndUnaryParselets;
