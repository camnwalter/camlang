#pragma once

#include "astnode.hpp"
#include "exceptions.hpp"
#include "symbol.hpp"
#include "token.hpp"

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

extern const std::unordered_map<TokenType, Precedence> precedenceMap;

class Parser {
private:
    std::vector<Token> tokens;
    TypeContext& ctx;
    SymbolTable& symbolTable;
    uint32_t index {0};

public:
    Parser(std::vector<Token>&& toks, TypeContext& types, SymbolTable& symTab) :
        tokens(std::move(toks)),
        ctx(types),
        symbolTable(symTab) {}

    [[nodiscard]] bool isAtEnd() const {
        return peek().tokenType == TokenType::Eof;
    }

    [[nodiscard]] Token peek() const {
        return tokens.at(index);
    }

    // gets current and moves to next
    Token advance() {
        if (!isAtEnd()) {
            next();
        }
        return prev();
    }

    [[nodiscard]] Token prev() const {
        return tokens.at(index - 1);
    }

    void next() {
        index++;
    }

    void before() {
        index--;
    }

    // if current token has type type, returns true. else returns false
    [[nodiscard]] bool check(std::same_as<TokenType> auto... types) const {
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
        if (auto res = precedenceMap.find(type); res != precedenceMap.end()) {
            return res->second;
        }

        // TODO: Verify this is what we actually want.
        return Precedence::Lowest;
    }

    std::unique_ptr<Expression> infix(std::unique_ptr<Expression>);
    std::unique_ptr<FunctionCall> functionCall(Token,
                                               std::unique_ptr<Expression>);
    std::unique_ptr<Expression> expression(Precedence precedence
                                           = Precedence::Lowest);

    std::unique_ptr<IdentifierNode> identifier();
    std::unique_ptr<AstNode> declaration();
    std::unique_ptr<Block>
        block(std::unordered_map<std::string_view, Symbol>&& symbolsToAdd = {});
    std::unique_ptr<IfStatement> ifStatement();
    std::unique_ptr<AstNode> statement();
    std::unique_ptr<File> file();

    std::unique_ptr<File> parse() {
        std::cout << "Parsing..." << std::endl;
        return file();
    }
};
