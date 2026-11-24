#include "../include/parse.hpp"

#include "../include/astnode.hpp"
#include "../include/exceptions.hpp"

#include <functional>

template <typename T>
ExpressionPtr createExpr(SourceLocation loc, T&& node) {
    return std::make_unique<ExpressionNode>(loc, std::forward<T>(node));
}

template <typename T>
StatementPtr createStmt(SourceLocation loc, T&& node) {
    return std::make_unique<StatementNode>(loc, std::forward<T>(node));
}

const std::unordered_map<TokenType, Precedence> precedenceMap = {
    {TokenType::Or, Precedence::Or},

    {TokenType::And, Precedence::And},

    {TokenType::BitOr, Precedence::BitOr},

    {TokenType::Xor, Precedence::Xor},

    {TokenType::BitAnd, Precedence::BitAnd},

    {TokenType::EqEq, Precedence::Equality},
    {TokenType::BangEq, Precedence::Equality},

    {TokenType::Less, Precedence::Comparison},
    {TokenType::LessEq, Precedence::Comparison},
    {TokenType::Greater, Precedence::Comparison},
    {TokenType::GreaterEq, Precedence::Comparison},

    {TokenType::LShift, Precedence::Shift},
    {TokenType::RShift, Precedence::Shift},

    {TokenType::Plus, Precedence::Plus},
    {TokenType::Minus, Precedence::Plus},

    {TokenType::Star, Precedence::Times},
    {TokenType::Slash, Precedence::Times},
    {TokenType::Mod, Precedence::Times},

    {TokenType::LParen, Precedence::Call},
};

CallNode Parser::functionCall(Token, ExpressionPtr left) {
    std::vector<ExpressionPtr> vec;
    if (!check(TokenType::RParen)) {
        do {
            vec.push_back(parseExpression());
        } while (match(TokenType::Comma));
    }

    expect(TokenType::RParen, ") expected after arguments");
    return CallNode {std::move(left), std::move(vec)};
}

ExpressionPtr Parser::infix(ExpressionPtr left) {
    auto precedence = nextPrecedence();

    auto tok = advance();

    if (precedence == Precedence::Call) {
        return createExpr(tokenLocation(tok),
                          functionCall(tok, std::move(left)));
    }

    auto op = tok.tokenType;
    auto rhs = parseExpression(precedence);

    return createExpr(tokenLocation(tok),
                      BinaryNode {op, std::move(left), std::move(rhs)});
}

ExpressionPtr Parser::parseExpression(Precedence precedence) {
    using UnaryParselet = std::function<ExpressionPtr(Token)>;

    static const UnaryParselet unary = [this](Token tok) {
        auto rhs = parseExpression(Precedence::Unary);
        return createExpr(tokenLocation(tok),
                          UnaryNode {tok.tokenType, std::move(rhs)});
    };

    static const std::unordered_map<TokenType, UnaryParselet>
        primaryAndUnaryParselets = {
            {
                TokenType::String,
                [](Token tok) {
                    return createExpr(
                        tokenLocation(tok),
                        StringLiteralNode {std::get<std::string>(tok.literal)}
                    );
                },
            },
            {
                TokenType::FNumber,
                [](Token tok) {
                    return createExpr(tokenLocation(tok),
                                      std::get<double>(tok.literal));
                },
            },
            {
                TokenType::INumber,
                [](Token tok) {
                    return createExpr(tokenLocation(tok),
                                      std::get<uint32_t>(tok.literal));
                },
            },
            {
                TokenType::Identifier,
                [this](Token tok) {
                    // We have already consumed the identifier, but
                    // identifier() expects that to be the next token.
                    before();
                    // TODO: Should we assign a type for this?
                    return createExpr(tokenLocation(tok), identifier());
                },
            },
            {
                TokenType::True,
                [](Token tok) { return createExpr(tokenLocation(tok), true); },
            },
            {
                TokenType::False,
                [](Token tok) { return createExpr(tokenLocation(tok), false); },
            },
            {
                TokenType::LParen,
                [this](Token tok) {
                    auto node = parseExpression();
                    expect(TokenType::RParen, "expected closing )");
                    node->loc = tokenLocation(tok);
                    return node;
                },
            },
            {TokenType::Plus, unary},
            {TokenType::Minus, unary},
            {TokenType::Bang, unary},
            {TokenType::Tilde, unary},
        };

    auto tok = peek();
    auto type = tok.tokenType;

    auto res = primaryAndUnaryParselets.find(type);
    if (res == primaryAndUnaryParselets.end()) {
        SyntaxError(std::format("Expected primary expression, got '{}'",
                                tok.lexeme),
                    tokenLocation(tok));
    }

    auto left = res->second(advance());

    while (precedence < nextPrecedence()) {
        left = infix(std::move(left));
    }

    return left;
}

TypeNode Parser::parseType() {
    // TODO: Allow array types
    auto typeTok = peek();
    return TypeNode {tokenLocation(typeTok), NamedTypeNode {identifier()}};
}

IdentifierNode Parser::identifier() {
    auto tok = expect(TokenType::Identifier, "identifier expected");
    return IdentifierNode {tok.lexeme};
}

StatementPtr Parser::declaration() {
    auto tok = peek();

    if (match(TokenType::Var)) {
        auto name = identifier();

        std::variant<std::monostate, TypeNode> type;
        if (match(TokenType::Colon)) {
            type = parseType();
        }

        if (match(TokenType::Eq)) {
            auto rhs = parseExpression();
            expect(TokenType::Semi,
                   "; expected at end of variable declaration");

            return createStmt(tokenLocation(tok),
                              VarDeclNode {std::move(name),
                                           std::move(type),
                                           std::move(rhs),
                                           false});
        } else {
            expect(TokenType::Semi,
                   "; expected at end of variable declaration");

            return createStmt(
                tokenLocation(tok),
                VarDeclNode {std::move(name), std::move(type), {}, false}
            );
        }
    }

    if (match(TokenType::Const)) {
        auto name = identifier();
        auto nameTok = prev();

        std::variant<std::monostate, TypeNode> type;
        if (match(TokenType::Colon)) {
            type = parseType();
        }

        if (match(TokenType::Eq)) {
            auto rhs = parseExpression();
            expect(TokenType::Semi,
                   "; expected at end of constant declaration");

            return createStmt(tokenLocation(tok),
                              VarDeclNode {std::move(name),
                                           std::move(type),
                                           std::move(rhs),
                                           true});
        } else {
            SyntaxError(std::format("Constant '{}' must be initialized",
                                    name.value),
                        tokenLocation(tok));
        }
    }

    if (match(TokenType::Fn)) {
        auto name = identifier();

        expect(TokenType::LParen, "( expected before function parameters");

        std::vector<ParamNode> params;
        if (!check(TokenType::RParen)) {
            do {
                auto param = identifier();

                expect(TokenType::Colon, "expected : after parameter name");
                auto type = parseType();

                params.push_back(ParamNode {std::move(param), std::move(type)});
            } while (match(TokenType::Comma));
        }

        expect(TokenType::RParen, ") expected after function parameters");

        auto returnType = parseType();

        return createStmt(tokenLocation(tok),
                          FunctionNode {std::move(name),
                                        std::move(params),
                                        std::move(returnType),
                                        block()});
    }

    SyntaxError("Declaration expected", tokenLocation(tok));
}

BlockNode Parser::block() {
    expect(TokenType::LBrace, "{ expected at start of block");

    BlockNode block;

    while (!check(TokenType::RBrace)) {
        block.nodes.push_back(statement());
    }

    expect(TokenType::RBrace, "} expected at end of block");
    return block;
}

StatementPtr Parser::ifStatement() {
    auto tok = prev(); // if already consumed

    auto cond = parseExpression();
    auto thenPart = block();
    IfNode::ElseClause elsePart;
    if (match(TokenType::Else)) {
        if (match(TokenType::If)) {
            elsePart = ifStatement();
        } else {
            elsePart = block();
        }
    }

    return createStmt(
        tokenLocation(tok),
        IfNode {std::move(cond), std::move(thenPart), std::move(elsePart)}
    );
}

StatementPtr Parser::statement() {
    if (check(TokenType::Var, TokenType::Const /* , TokenType::Fn */)) {
        return declaration();
    }

    auto tok = peek();

    if (match(TokenType::While)) {
        auto cond = parseExpression();
        auto inside = block();

        return createStmt(tokenLocation(tok),
                          WhileNode {std::move(cond), std::move(inside)});
    }

    if (match(TokenType::If)) {
        return ifStatement();
    }

    size_t prevIndex = index;

    // TODO: This only works for identifier += ...; Need to add support
    // for indices etc
    if (match(TokenType::Identifier)
        && check(TokenType::Eq,
                 TokenType::PlusEq,
                 TokenType::MinusEq,
                 TokenType::StarEq,
                 TokenType::SlashEq)) {
        before();
        auto lhs = parseExpression();
        auto tok = advance();
        auto op = tok.tokenType;
        auto rhs = parseExpression();

        expect(TokenType::Semi, "; expected");

        return createStmt(
            tokenLocation(tok),
            AssignmentNode {std::move(op), std::move(lhs), std::move(rhs)}
        );
    } else {
        // unconsume the identifier
        index = prevIndex;
    }

    if (match(TokenType::Return)) {
        auto ret
            = createStmt(tokenLocation(tok), ReturnNode {parseExpression()});
        expect(TokenType::Semi, "; expected after return statement");
        return ret;
    }

    if (match(TokenType::LBrace)) {
        before();
        return createStmt(tokenLocation(tok), block());
    }

    auto expr = parseExpression();
    expect(TokenType::Semi, "; expected after expression");
    return createStmt(tokenLocation(tok),
                      ExpressionStatementNode {std::move(expr)});
}
