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
        } while (match(TokenType::Comma) && !check(TokenType::RParen));
    }

    expect(TokenType::RParen, ") expected after arguments");
    return std::make_unique<FunctionCall>(std::move(left), std::move(vec));
}

std::unique_ptr<Expression> Parser::infix(std::unique_ptr<Expression> left) {
    auto precedence = nextPrecedence();

    if (precedence == Precedence::Call) {
        return functionCall(advance(), std::move(left));
    }

    return std::make_unique<BinaryOperator>(std::move(left),
                                            advance(),
                                            expression(precedence));
}

std::unique_ptr<Expression> Parser::expression(Precedence precedence) {
    using UnaryParselet = std::function<std::unique_ptr<Expression>(Token)>;

    static const UnaryParselet unary = [this](Token tok) {
        return std::make_unique<UnaryOperator>(tok,
                                               expression(Precedence::Unary));
    };

    static const UnaryParselet ifExpression = [this](Token) {
        auto cond = expression();
        auto thenPart = block();
        expect(TokenType::Else, "else is required for if expression");

        std::unique_ptr<Expression> elsePart;
        if (match(TokenType::If)) {
            elsePart = ifExpression(prev());
        } else {
            elsePart = block();
        }

        return std::make_unique<IfExpression>(std::move(cond),
                                              std::move(thenPart),
                                              std::move(elsePart));
    };

    static const std::unordered_map<TokenType, UnaryParselet>
        primaryAndUnaryParselets = {
            {
                TokenType::String,
                [this](Token tok) {
                    auto node = std::make_unique<StringNode>(
                        std::get<std::string>(tok.literal)
                    );
                    node->type = ctx.get("string");
                    return node;
                },
            },
            {
                TokenType::FNumber,
                [this](Token tok) {
                    auto node = std::make_unique<DoubleNode>(
                        std::get<double>(tok.literal)
                    );
                    node->type = ctx.get("f64");
                    return node;
                },
            },
            {
                TokenType::INumber,
                [this](Token tok) {
                    auto node = std::make_unique<IntNode>(
                        std::get<uint32_t>(tok.literal)
                    );
                    node->type = ctx.get("i32");
                    return node;
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
                [this](Token) {
                    auto node = std::make_unique<BoolNode>(true);
                    node->type = ctx.get("bool");
                    return node;
                },
            },
            {
                TokenType::False,
                [this](Token) {
                    auto node = std::make_unique<BoolNode>(false);
                    node->type = ctx.get("bool");
                    return node;
                },
            },
            {
                TokenType::LParen,
                [this](Token) {
                    auto node = expression();
                    expect(TokenType::RParen, "expected closing )");
                    return node;
                },
            },
            {TokenType::If, ifExpression},
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

std::unique_ptr<AstNode> Parser::declaration() {
    if (match(TokenType::Var)) {
        auto name = identifier();
        auto nameTok = prev();

        if (match(TokenType::Eq)) {
            auto rhs = expression();
            expect(TokenType::Semi,
                   "; expected at end of variable declaration");

            if (!symbolTable.insert(name->value,
                                    {.kind = SymbolKind::Var,
                                     .datatype = rhs->type})) {
                SyntaxError(std::format("Redefinition of variable '{}'",
                                        name->value),
                            nameTok.line,
                            nameTok.column);
            }

            return std::make_unique<VarDeclaration>(std::move(name),
                                                    std::move(rhs),
                                                    false);
        } else {
            expect(TokenType::Colon,
                   "uninitialized variable must have type qualifier");

            auto typeName = identifier();
            expect(TokenType::Semi,
                   "; expected at end of variable declaration");

            if (!symbolTable.insert(
                    name->value,
                    {.kind = SymbolKind::Var,
                     .datatype = ctx.getOrInsertDummy(typeName->value)}
                )) {
                SyntaxError(std::format("Redefinition of variable '{}'",
                                        name->value),
                            nameTok.line,
                            nameTok.column);
            }

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

            if (!symbolTable.insert(name->value,
                                    {.kind = SymbolKind::Const,
                                     .datatype = rhs->type})) {
                SyntaxError(std::format("Redefinition of constant '{}'",
                                        name->value),
                            nameTok.line,
                            nameTok.column);
            }
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

        std::vector<std::unique_ptr<IdentifierNode>> params;
        std::vector<const Type*> paramTypes;
        std::unordered_map<std::string_view, Symbol> additions;

        if (!check(TokenType::RParen)) {
            do {
                auto param = identifier();

                expect(TokenType::Colon, "expected : after parameter name");
                auto typeName = identifier();

                auto paramType = ctx.getOrInsertDummy(typeName->value);
                paramTypes.push_back(paramType);

                Symbol sym {.kind = SymbolKind::Const, .datatype = paramType};
                additions.insert({param->value, sym});

                params.push_back(std::move(param));
            } while (match(TokenType::Comma));
        }

        expect(TokenType::RParen, ") expected after function parameters");

        auto returnType = identifier();

        symbolTable.insert(
            name->value,
            {.kind = SymbolKind::Function,
             .datatype
             = ctx.createFunction(name->value,
                                  ctx.getOrInsertDummy(returnType->value),
                                  paramTypes)}
        );

        auto body = block(std::move(additions));

        return std::make_unique<Function>(std::move(name),
                                          std::move(params),
                                          std::move(returnType),
                                          std::move(body));
    }

    SyntaxError("Declaration expected", prev().line, prev().column);
}

std::unique_ptr<Block>
    Parser::block(std::unordered_map<std::string_view, Symbol>&& symbolsToAdd) {
    expect(TokenType::LBrace, "{ expected at start of block");

    auto block = std::make_unique<Block>();

    symbolTable.enterScope();
    symbolTable.addMany(std::move(symbolsToAdd));

    while (!check(TokenType::RBrace)) {
        block->addNode(statement());
    }

    expect(TokenType::RBrace, "} expected at end of block");
    symbolTable.exitScope();
    return block;
}

std::unique_ptr<IfStatement> Parser::ifStatement() {
    auto cond = expression();
    auto thenPart = block();
    std::unique_ptr<AstNode> elsePart = nullptr;
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

std::unique_ptr<AstNode> Parser::statement() {
    if (check(TokenType::Var, TokenType::Const, TokenType::Fn)) {
        return declaration();
    }

    if (match(TokenType::While)) {
        auto cond = expression();
        auto inside = block();
        return std::make_unique<While>(std::move(cond), std::move(inside));
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
        auto assignment = std::make_unique<Assignment>(identifier(),
                                                       advance(),
                                                       expression());
        expect(TokenType::Semi, "; expected");
        return assignment;
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
        file->addStatement(declaration());
    }

    return file;
}
