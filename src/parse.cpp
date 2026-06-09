#include "../include/parse.hpp"

#include "../include/astnode.hpp"
#include "../include/exceptions.hpp"

std::unique_ptr<IdentifierNode> Parser::identifier() {
    auto tok = expect(TokenType::Identifier, "identifier expected");
    return std::make_unique<IdentifierNode>(std::string(tok.lexeme));
}

std::vector<std::unique_ptr<Expression>> Parser::args() {
    std::vector<std::unique_ptr<Expression>> vec;
    if (!check(TokenType::RParen)) {
        do {
            vec.push_back(expression());
        } while (match(TokenType::Comma) && !check(TokenType::RParen));
    }

    expect(TokenType::RParen, ") expected after arguments");
    return vec;
}

std::unique_ptr<Expression> Parser::primary() {
    // Todo: Use a string pool, int pool, identifier pool,
    // and only 1 true/false node
    if (match(TokenType::String)) {
        return std::make_unique<StringNode>(
            std::get<std::string>(prev().literal)
        );
    }
    if (match(TokenType::FNumber)) {
        return std::make_unique<DoubleNode>(std::get<double>(prev().literal));
    }
    if (match(TokenType::INumber)) {
        return std::make_unique<IntNode>(std::get<uint32_t>(prev().literal));
    }
    if (check(TokenType::Identifier)) {
        return identifier();
    }
    if (match(TokenType::True)) {
        return std::make_unique<BoolNode>(true);
    }
    if (match(TokenType::False)) {
        return std::make_unique<BoolNode>(false);
    }
    if (match(TokenType::LParen)) {
        auto node = expression();
        expect(TokenType::RParen, "expected closing )");
        return node;
    }

    SyntaxError(std::format("Expected expression, got '{}'", peek().lexeme),
                peek().line,
                peek().column);
}

std::unique_ptr<Expression> Parser::call() {
    auto node = primary();
    while (match(TokenType::LParen)) {
        node = std::make_unique<FunctionCall>(std::move(node), args());
    }
    return node;
}

std::unique_ptr<Expression> Parser::unary() {
    if (match(TokenType::Minus, TokenType::Plus)) {
        return std::make_unique<UnaryOperator>(prev(), call());
    }

    if (match(TokenType::Bang, TokenType::Tilde)) {
        return std::make_unique<UnaryOperator>(prev(), unary());
    }

    return call();
}

std::unique_ptr<Expression> Parser::factor() {
    auto node = unary();
    while (match(TokenType::Star, TokenType::Slash, TokenType::Mod)) {
        node =
            std::make_unique<BinaryOperator>(std::move(node), prev(), unary());
    }

    return node;
}

std::unique_ptr<Expression> Parser::term() {
    auto node = factor();
    while (match(TokenType::Plus, TokenType::Minus)) {
        node =
            std::make_unique<BinaryOperator>(std::move(node), prev(), factor());
    }

    return node;
}

std::unique_ptr<Expression> Parser::shifts() {
    auto node = term();
    while (match(TokenType::LShift, TokenType::RShift)) {
        node =
            std::make_unique<BinaryOperator>(std::move(node), prev(), term());
    }

    return node;
}

std::unique_ptr<Expression> Parser::comparison() {
    auto node = shifts();
    while (match(TokenType::LAngle,
                 TokenType::LAngleEq,
                 TokenType::RAngle,
                 TokenType::RAngleEq)) {
        node =
            std::make_unique<BinaryOperator>(std::move(node), prev(), shifts());
    }
    return node;
}

std::unique_ptr<Expression> Parser::equality() {
    auto node = comparison();
    while (match(TokenType::EqEq, TokenType::BangEq)) {
        node = std::make_unique<BinaryOperator>(std::move(node),
                                                prev(),
                                                comparison());
    }
    return node;
}

std::unique_ptr<Expression> Parser::bitandexpr() {
    auto node = equality();
    while (match(TokenType::BitAnd)) {
        node = std::make_unique<BinaryOperator>(std::move(node),
                                                prev(),
                                                equality());
    }
    return node;
}

std::unique_ptr<Expression> Parser::xorexpr() {
    auto node = bitandexpr();
    while (match(TokenType::Xor)) {
        node = std::make_unique<BinaryOperator>(std::move(node),
                                                prev(),
                                                bitandexpr());
    }
    return node;
}

std::unique_ptr<Expression> Parser::bitorexpr() {
    auto node = xorexpr();
    while (match(TokenType::BitOr)) {
        node = std::make_unique<BinaryOperator>(std::move(node),
                                                prev(),
                                                xorexpr());
    }
    return node;
}

std::unique_ptr<Expression> Parser::andexpr() {
    auto node = bitorexpr();
    while (match(TokenType::And)) {
        node = std::make_unique<BinaryOperator>(std::move(node),
                                                prev(),
                                                bitorexpr());
    }
    return node;
}

std::unique_ptr<Expression> Parser::orexpr() {
    auto node = andexpr();
    while (match(TokenType::Or)) {
        node = std::make_unique<BinaryOperator>(std::move(node),
                                                prev(),
                                                andexpr());
    }
    return node;
}

std::unique_ptr<Expression> Parser::expression() {
    auto node = orexpr();

    while (match(TokenType::Eq,
                 TokenType::PlusEq,
                 TokenType::MinusEq,
                 TokenType::StarEq,
                 TokenType::SlashEq)) {
        node =
            std::make_unique<Assignment>(std::move(node), prev(), expression());
    }

    return node;
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
    auto expr = expression();
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

std::unique_ptr<AstNode> Parser::statement() {
    if (check(TokenType::LBrace)) {
        return block({});
    }

    if (match(TokenType::If)) {
        auto cond = expression();
        auto then = block({});

        std::unique_ptr<Block> elsePart = nullptr;
        if (match(TokenType::Else)) {
            elsePart = block({});
        }
        return std::make_unique<If>(std::move(cond),
                                    std::move(then),
                                    std::move(elsePart));
    }

    if (match(TokenType::While)) {
        auto cond = expression();
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
