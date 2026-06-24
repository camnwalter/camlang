#include "../include/parse.hpp"

#include "../include/astnode.hpp"
#include "../include/exceptions.hpp"

#include <functional>

const std::unordered_map<TokenType, Precedence> precedenceMap = {
    {TokenType::Or, Precedence::Or},

    {TokenType::And, Precedence::And},

    {TokenType::BitOr, Precedence::BitOr},

    {TokenType::Xor, Precedence::Xor},

    {TokenType::BitAnd, Precedence::BitAnd},

    {TokenType::EqEq, Precedence::Equality},
    {TokenType::BangEq, Precedence::Equality},

    {TokenType::LAngle, Precedence::Comparison},
    {TokenType::LAngleEq, Precedence::Comparison},
    {TokenType::RAngle, Precedence::Comparison},
    {TokenType::RAngleEq, Precedence::Comparison},

    {TokenType::LShift, Precedence::Shift},
    {TokenType::RShift, Precedence::Shift},

    {TokenType::Plus, Precedence::Plus},
    {TokenType::Minus, Precedence::Plus},

    {TokenType::Star, Precedence::Times},
    {TokenType::Slash, Precedence::Times},
    {TokenType::Mod, Precedence::Times},

    {TokenType::LParen, Precedence::Call},
};

std::unique_ptr<FunctionCall>
    Parser::functionCall(Token, std::unique_ptr<Expression> left) {
    std::vector<std::unique_ptr<Expression>> vec;
    if (!check(TokenType::RParen)) {
        do {
            vec.push_back(expression());
        } while (match(TokenType::Comma));
    }

    expect(TokenType::RParen, ") expected after arguments");
    return std::make_unique<FunctionCall>(std::move(left), std::move(vec));
}

std::unique_ptr<Expression> Parser::infix(std::unique_ptr<Expression> left) {
    auto precedence = nextPrecedence();

    if (precedence == Precedence::Call) {
        return functionCall(advance(), std::move(left));
    }

    // TODO: fill in types
    auto op = advance();
    auto rhs = expression(precedence);

    return std::make_unique<BinaryOperator>(std::move(left),
                                            std::move(op),
                                            std::move(rhs));
}

std::unique_ptr<Expression> Parser::expression(Precedence precedence) {
    using UnaryParselet = std::function<std::unique_ptr<Expression>(Token)>;

    static const UnaryParselet unary = [this](Token tok) {
        auto rhs = expression(Precedence::Unary);
        return std::make_unique<UnaryOperator>(tok, std::move(rhs));
    };

    static const std::unordered_map<TokenType, UnaryParselet>
        primaryAndUnaryParselets = {
            {
                TokenType::String,
                [](Token tok) {
                    return std::make_unique<StringNode>(
                        std::get<std::string>(tok.literal)
                    );
                },
            },
            {
                TokenType::FNumber,
                [](Token tok) {
                    return std::make_unique<DoubleNode>(
                        std::get<double>(tok.literal)
                    );
                },
            },
            {
                TokenType::INumber,
                [](Token tok) {
                    return std::make_unique<IntNode>(
                        std::get<uint32_t>(tok.literal)
                    );
                },
            },
            {
                TokenType::Identifier,
                [this](Token) {
                    // We have already consumed the identifier, but
                    // identifier() expects that to be the next token.
                    before();
                    // TODO: Should we assign a type for this?
                    return identifier();
                },
            },
            {
                TokenType::True,
                [](Token) { return std::make_unique<BoolNode>(true); },
            },
            {
                TokenType::False,
                [](Token) { return std::make_unique<BoolNode>(false); },
            },
            {
                TokenType::LParen,
                [this](Token) {
                    auto node = expression();
                    expect(TokenType::RParen, "expected closing )");
                    return node;
                },
            },
            {TokenType::Plus, unary},
            {TokenType::Minus, unary},
            {TokenType::Bang, unary},
            {TokenType::Tilde, unary},
        };

    auto type = peek().tokenType;

    auto res = primaryAndUnaryParselets.find(type);
    if (res == primaryAndUnaryParselets.end()) {
        SyntaxError(std::format("Expected primary expression, got '{}'",
                                peek().lexeme),
                    peek().line,
                    peek().column);
    }

    auto left = res->second(advance());

    while (precedence < nextPrecedence()) {
        left = infix(std::move(left));
    }

    return left;
}

std::unique_ptr<IdentifierNode> Parser::identifier() {
    auto tok = expect(TokenType::Identifier, "identifier expected");
    return std::make_unique<IdentifierNode>(tok.lexeme);
}

std::unique_ptr<Statement> Parser::declaration() {
    if (match(TokenType::Var)) {
        auto name = identifier();

        if (match(TokenType::Eq)) {
            auto rhs = expression();
            expect(TokenType::Semi,
                   "; expected at end of variable declaration");

            return std::make_unique<VarDeclaration>(std::move(name),
                                                    std::move(rhs),
                                                    false);
        } else {
            expect(TokenType::Colon,
                   "uninitialized variable must have type qualifier");

            auto typeName = identifier();
            expect(TokenType::Semi,
                   "; expected at end of variable declaration");

            return std::make_unique<VarDeclaration>(std::move(name),
                                                    nullptr,
                                                    false);
        }
    }

    if (match(TokenType::Const)) {
        auto name = identifier();
        auto nameTok = prev();

        if (match(TokenType::Eq)) {
            auto rhs = expression();
            expect(TokenType::Semi,
                   "; expected at end of constant declaration");

            return std::make_unique<VarDeclaration>(std::move(name),
                                                    std::move(rhs),
                                                    true);
        } else {
            SyntaxError(std::format("Constant '{}' must be initialized",
                                    name->value),
                        peek().line,
                        peek().column);
        }
    }

    if (match(TokenType::Fn)) {
        auto name = identifier();

        expect(TokenType::LParen, "( expected before function parameters");

        std::vector<std::pair<std::unique_ptr<IdentifierNode>,
                              std::unique_ptr<IdentifierNode>>>
            params;
        if (!check(TokenType::RParen)) {
            do {
                auto param = identifier();

                expect(TokenType::Colon, "expected : after parameter name");
                auto typeName = identifier();

                params.push_back({std::move(param), std::move(typeName)});
            } while (match(TokenType::Comma));
        }

        expect(TokenType::RParen, ") expected after function parameters");

        auto returnType = identifier();

        return std::make_unique<Function>(
            std::move(name), std::move(params), std::move(returnType), block()
        );
    }

    SyntaxError("Declaration expected", prev().line, prev().column);
}

std::unique_ptr<Block> Parser::block() {
    expect(TokenType::LBrace, "{ expected at start of block");

    auto block = std::make_unique<Block>();

    while (!check(TokenType::RBrace)) {
        block->addNode(statement());
    }

    expect(TokenType::RBrace, "} expected at end of block");
    return block;
}

std::unique_ptr<IfStatement> Parser::ifStatement() {
    auto cond = expression();
    auto thenPart = block();
    std::unique_ptr<Statement> elsePart = nullptr;
    if (match(TokenType::Else)) {
        if (match(TokenType::If)) {
            elsePart = ifStatement();
        } else {
            elsePart = block();
        }
    }

    return std::make_unique<IfStatement>(std::move(cond),
                                         std::move(thenPart),
                                         std::move(elsePart));
}

std::unique_ptr<Statement> Parser::statement() {
    // TODO: allow nested functions
    if (check(TokenType::Var, TokenType::Const /* , TokenType::Fn */)) {
        return declaration();
    }

    if (match(TokenType::While)) {
        auto cond = expression();
        auto inside = block();

        auto stmt = std::make_unique<While>(std::move(cond), std::move(inside));
        return stmt;
    }

    if (match(TokenType::If)) {
        return ifStatement();
    }

    size_t prevIndex = index;

    if (match(TokenType::Identifier)
        && check(TokenType::Eq,
                 TokenType::PlusEq,
                 TokenType::MinusEq,
                 TokenType::StarEq,
                 TokenType::SlashEq)) {
        before();
        auto lhs = identifier();
        auto op = advance();
        auto rhs = expression();

        expect(TokenType::Semi, "; expected");

        return std::make_unique<Assignment>(std::move(lhs),
                                            std::move(op),
                                            std::move(rhs));
    } else {
        // unconsume the identifier
        index = prevIndex;
    }

    if (match(TokenType::Return)) {
        auto ret = std::make_unique<ReturnStatement>(expression());
        expect(TokenType::Semi, "; expected after return statement");
        return ret;
    }

    if (match(TokenType::LBrace)) {
        before();
        return block();
    }

    auto expr = expression();
    expect(TokenType::Semi, "; expected after expression");
    return expr;
}

std::unique_ptr<File> Parser::file() {
    auto file = std::make_unique<File>();
    while (!isAtEnd()) {
        file->addDeclaration(declaration());
    }

    return file;
}
