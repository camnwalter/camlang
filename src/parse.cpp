#include "../include/parse.hpp"

#include "../include/astnode.hpp"
#include "../include/exceptions.hpp"

std::unordered_map<TokenType, Precedence> precedenceMap = {
    {TokenType::Eq, Precedence::Assignment},
    {TokenType::PlusEq, Precedence::Assignment},
    {TokenType::MinusEq, Precedence::Assignment},
    {TokenType::StarEq, Precedence::Assignment},
    {TokenType::SlashEq, Precedence::Assignment},

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

UnaryParselet unary = [](Parser* p, Token tok) {
    return std::make_unique<UnaryOperator>(
        tok, p->parseExpression(Precedence::Unary)
    );
};

std::unordered_map<TokenType, UnaryParselet> primaryAndUnaryParselets = {
    {
        TokenType::String,
        [](Parser*, Token tok) {
            return std::make_unique<StringNode>(
                std::get<std::string>(tok.literal)
            );
        },
    },
    {
        TokenType::FNumber,
        [](Parser*, Token tok) {
            return std::make_unique<DoubleNode>(std::get<double>(tok.literal));
        },
    },
    {
        TokenType::INumber,
        [](Parser*, Token tok) {
            return std::make_unique<IntNode>(std::get<uint32_t>(tok.literal));
        },
    },
    {
        TokenType::Identifier,
        [](Parser* p, Token) {
            // We have already consumed the identifier, but p->identifier()
            // expects that to be the next token
            p->before();
            return p->identifier();
        },
    },
    {
        TokenType::True,
        [](Parser*, Token) { return std::make_unique<BoolNode>(true); },
    },
    {
        TokenType::False,
        [](Parser*, Token) { return std::make_unique<BoolNode>(false); },
    },
    {
        TokenType::LParen,
        [](Parser* p, Token) {
            auto node = p->parseExpression(Precedence::Lowest);
            p->expect(TokenType::RParen, "expected closing )");
            return node;
        },
    },
    {TokenType::Plus, unary},
    {TokenType::Minus, unary},
    {TokenType::Bang, unary},
    {TokenType::Tilde, unary},
};

BinaryParselet binaryOperatorParselet(Precedence precedence) {
    return
        [precedence](Parser* p, Token tok, std::unique_ptr<Expression> left) {
            return std::make_unique<BinaryOperator>(
                std::move(left), tok, p->parseExpression(precedence)
            );
        };
}

std::unique_ptr<FunctionCall>
    Parser::parseFunctionCall(Token, std::unique_ptr<Expression> left) {
    std::vector<std::unique_ptr<Expression>> vec;
    if (!check(TokenType::RParen)) {
        do {
            vec.push_back(parseExpression(Precedence::Lowest));
        } while (match(TokenType::Comma) && !check(TokenType::RParen));
    }

    expect(TokenType::RParen, ") expected after arguments");
    return std::make_unique<FunctionCall>(std::move(left), std::move(vec));
}

std::unique_ptr<Assignment>
    Parser::parseAssignment(Token tok, std::unique_ptr<Expression> left) {
    // Assignment is right-associative, so we take the precedence directly below
    // assignment, which happens to be Lowest.
    return std::make_unique<Assignment>(std::move(left),
                                        tok,
                                        parseExpression(Precedence::Lowest));
}

std::unique_ptr<Expression>
    Parser::parseInfix(std::unique_ptr<Expression> left) {
    auto precedence = nextPrecedence();

    if (precedence == Precedence::Call) {
        return parseFunctionCall(advance(), std::move(left));
    } else if (precedence == Precedence::Assignment) {
        return parseAssignment(advance(), std::move(left));
    }

    return binaryOperatorParselet(precedence)(this, advance(), std::move(left));
}

std::unique_ptr<Expression> Parser::parseExpression(Precedence precedence) {
    auto type = peek().tokenType;

    if (!primaryAndUnaryParselets.contains(type)) {
        SyntaxError(std::format("Expected primary expression, got '{}'",
                                peek().lexeme),
                    peek().line,
                    peek().column);
    }

    auto left = primaryAndUnaryParselets[type](this, advance());

    while (precedence < nextPrecedence()) {
        left = parseInfix(std::move(left));
    }

    return left;
}

std::unique_ptr<IdentifierNode> Parser::identifier() {
    auto tok = expect(TokenType::Identifier, "identifier expected");
    return std::make_unique<IdentifierNode>(std::string(tok.lexeme));
}

std::unique_ptr<AstNode> Parser::declaration() {
    if (match(TokenType::Fn)) {
        auto name = identifier();
        symbolTable->insert(name->value,
                            {.declarationType = DeclarationType::Function});

        expect(TokenType::LParen, "( expected before function parameters");

        std::vector<std::unique_ptr<IdentifierNode>> params;
        if (!check(TokenType::RParen)) {
            do {
                params.push_back(identifier());
            } while (match(TokenType::Comma) && !check(TokenType::RParen));
        }

        expect(TokenType::RParen, ") expected after function parameters");

        std::unordered_map<std::string, Symbol> additions;
        for (auto&& param : params) {
            additions.insert({param->value,
                              {.declarationType = DeclarationType::Var}});
        }

        auto body = block(std::move(additions));

        return std::make_unique<Function>(std::move(name),
                                          std::move(params),
                                          std::move(body));
    }

    if (match(TokenType::Var)) {
        auto name = identifier();
        if (match(TokenType::Eq)) {
            auto rhs = expressionStatement();
            symbolTable->insert(name->value,
                                {.declarationType = DeclarationType::Var});
            return std::make_unique<VarDeclaration>(std::move(name),
                                                    std::move(rhs),
                                                    false);
        } else {
            // Todo: specify type?
            expect(TokenType::Semi,
                   "; expected at end of variable declaration");
            symbolTable->insert(name->value,
                                {.declarationType = DeclarationType::Var});
            return std::make_unique<VarDeclaration>(std::move(name),
                                                    nullptr,
                                                    false);
        }
    }

    if (match(TokenType::Const)) {
        auto name = identifier();
        if (match(TokenType::Eq)) {
            auto rhs = expressionStatement();
            symbolTable->insert(name->value,
                                {.declarationType = DeclarationType::Const});
            return std::make_unique<VarDeclaration>(std::move(name),
                                                    std::move(rhs),
                                                    true);
        } else {
            SyntaxError("Constants must be initialized",
                        peek().line,
                        peek().column);
        }
    }

    return statement();
}

std::unique_ptr<ExpressionStatement> Parser::expressionStatement() {
    auto expr = parseExpression(Precedence::Lowest);
    expect(TokenType::Semi, "; expected at end of statement");
    return std::make_unique<ExpressionStatement>(std::move(expr));
}

std::unique_ptr<Block>
    Parser::block(std::unordered_map<std::string, Symbol>&& symbolsToAdd) {
    expect(TokenType::LBrace, "{ expected at start of block");
    auto block = std::make_unique<Block>();
    symbolTable->enterScope();

    symbolTable->addMany(std::move(symbolsToAdd));

    while (!check(TokenType::RBrace)) {
        block->addNode(declaration());
    }

    expect(TokenType::RBrace, "} expected at end of block");
    symbolTable->exitScope();
    return block;
}

std::unique_ptr<If> Parser::ifStatement() {
    auto cond = parseExpression(Precedence::Lowest);
    auto then = block({});

    std::unique_ptr<AstNode> elsePart = nullptr;
    if (match(TokenType::Else)) {
        if (match(TokenType::If)) {
            elsePart = ifStatement();
        } else {
            elsePart = block({});
        }
    }

    return std::make_unique<If>(std::move(cond),
                                std::move(then),
                                std::move(elsePart));
}

std::unique_ptr<AstNode> Parser::statement() {
    if (check(TokenType::LBrace)) {
        return block({});
    }

    if (match(TokenType::If)) {
        return ifStatement();
    }

    if (match(TokenType::While)) {
        auto cond = parseExpression(Precedence::Lowest);
        auto inside = block({});
        return std::make_unique<While>(std::move(cond), std::move(inside));
    }

    if (match(TokenType::Return)) {
        return std::make_unique<ReturnStatement>(expressionStatement());
    }

    if (match(TokenType::Semi)) {
        return std::make_unique<EmptyStatement>();
    }

    return expressionStatement();
}

std::unique_ptr<File> Parser::file() {
    auto file = std::make_unique<File>();
    while (!isAtEnd()) {
        file->addStatement(declaration());
    }

    return file;
}
