#include "../include/parse.hpp"

#include "../include/astnode.hpp"
#include "../include/exceptions.hpp"

IdentifierNode* Parser::Parser::identifier() {
    auto tok = expect(TokenType::Identifier, "identifier expected");
    return new IdentifierNode(std::get<std::string>(tok->value));
}

std::vector<AstNode*> Parser::args() {
    std::vector<AstNode*> vec;
    if (!check(TokenType::RParen)) {
        do {
            vec.push_back(expression());
        } while (match(TokenType::Comma) && !check(TokenType::RParen));
    }

    expect(TokenType::RParen, ") expected after arguments");
    return vec;
}

AstNode* Parser::primary() {
    if (match(TokenType::String)) {
        return new StringNode(std::get<std::string>(prev()->value));
    }
    if (match(TokenType::FNumber)) {
        return new DoubleNode(std::get<double>(prev()->value));
    }
    if (match(TokenType::INumber)) {
        return new IntNode(std::get<uint32_t>(prev()->value));
    }
    if (check(TokenType::Identifier)) {
        return identifier();
    }
    if (match(TokenType::True)) {
        return new BoolNode(true);
    }
    if (match(TokenType::False)) {
        return new BoolNode(false);
    }
    if (match(TokenType::LParen)) {
        auto node = expression();
        expect(TokenType::RParen, "expected closing )");
        return node;
    }

    SyntaxError(std::format("Expected expression, got '{}'", peek()->lexeme),
                peek()->lineno,
                peek()->colno);
}

AstNode* Parser::call() {
    auto node = primary();
    while (match(TokenType::LParen)) {
        node = new FunctionCall(node, args());
    }
    return node;
}

AstNode* Parser::exponent() {
    AstNode* node = call();
    while (match(TokenType::Caret)) {
        node = new BinaryOperator(node, prev(), call());
    }
    return node;
}

AstNode* Parser::unary() {
    if (match(TokenType::Minus, TokenType::Plus)) {
        return new UnaryOperator(prev(), exponent());
    }

    if (match(TokenType::Not, TokenType::BitNot)) {
        return new UnaryOperator(prev(), unary());
    }

    return exponent();
}

AstNode* Parser::factor() {
    AstNode* expr = unary();
    while (match(TokenType::Star, TokenType::Slash, TokenType::Mod)) {
        expr = new BinaryOperator(expr, prev(), unary());
    }

    return expr;
}

AstNode* Parser::term() {
    AstNode* expr = factor();
    while (match(TokenType::Plus, TokenType::Minus)) {
        expr = new BinaryOperator(expr, prev(), factor());
    }

    return expr;
}

AstNode* Parser::shifts() {
    AstNode* node = term();
    while (match(TokenType::LShift, TokenType::RShift)) {
        node = new BinaryOperator(node, prev(), term());
    }

    return node;
}

AstNode* Parser::comparison() {
    AstNode* node = shifts();
    while (match(TokenType::LT, TokenType::LE, TokenType::GT, TokenType::GE)) {
        node = new BinaryOperator(node, prev(), shifts());
    }
    return node;
}

AstNode* Parser::equality() {
    AstNode* node = comparison();
    while (match(TokenType::EqEq, TokenType::Ne)) {
        node = new BinaryOperator(node, prev(), comparison());
    }
    return node;
}

AstNode* Parser::bitandexpr() {
    AstNode* node = equality();
    while (match(TokenType::BitAnd)) {
        node = new BinaryOperator(node, prev(), equality());
    }
    return node;
}

AstNode* Parser::xorexpr() {
    AstNode* node = bitandexpr();
    while (match(TokenType::Xor)) {
        node = new BinaryOperator(node, prev(), bitandexpr());
    }
    return node;
}

AstNode* Parser::bitorexpr() {
    AstNode* node = xorexpr();
    while (match(TokenType::BitOr)) {
        node = new BinaryOperator(node, prev(), xorexpr());
    }
    return node;
}

AstNode* Parser::andexpr() {
    AstNode* node = bitorexpr();
    while (match(TokenType::And)) {
        node = new BinaryOperator(node, prev(), bitorexpr());
    }
    return node;
}

AstNode* Parser::orexpr() {
    AstNode* node = andexpr();
    while (match(TokenType::Or)) {
        node = new BinaryOperator(node, prev(), andexpr());
    }
    return node;
}

AstNode* Parser::expression() {
    auto node = orexpr();

    while (match(TokenType::Eq,
                 TokenType::PlusEq,
                 TokenType::MinusEq,
                 TokenType::StarEq,
                 TokenType::SlashEq)) {
        node = new Assignment(node, prev(), expression());
    }

    return node;
}

AstNode* Parser::declaration() {
    if (match(TokenType::Fn)) {
        auto name = identifier();
        expect(TokenType::LParen, "( expected before function parameters");

        std::vector<IdentifierNode*> params;
        if (!check(TokenType::RParen)) {
            do {
                params.push_back(identifier());
            } while (match(TokenType::Comma) && !check(TokenType::RParen));
        }

        expect(TokenType::RParen, ") expected after function parameters");

        auto body = block();

        return new Function(name, params, body);
    }

    if (match(TokenType::Var)) {
        auto name = identifier();
        if (match(TokenType::Eq)) {
            auto rhs = expressionStatement();
            return new VarDeclaration(name, rhs);
        } else {
            // Todo: specify type?
            expect(TokenType::Semi,
                   "; expected at end of variable declaration");
            return new VarDeclaration(name, nullptr);
        }
    }

    if (match(TokenType::Const)) {
        auto name = identifier();
        if (match(TokenType::Eq)) {
            auto rhs = expressionStatement();
            return new ConstDeclaration(name, rhs);
        } else {
            SyntaxError("Constants must be initialized",
                        peek()->lineno,
                        peek()->colno);
        }
    }

    return statement();
}

ExpressionStatement* Parser::expressionStatement() {
    auto expr = expression();
    expect(TokenType::Semi, "; expected at end of statement");
    return new ExpressionStatement(expr);
}

Block* Parser::block() {
    expect(TokenType::LBrace, "{ expected at start of block");
    auto block = new Block();

    while (!check(TokenType::RBrace)) {
        block->addNode(declaration());
    }

    expect(TokenType::RBrace, "} expected at end of block");
    return block;
}

AstNode* Parser::statement() {
    if (check(TokenType::LBrace)) {
        return block();
    }

    if (match(TokenType::If)) {
        auto cond = expression();
        auto then = block();

        Block* elsePart = nullptr;
        if (match(TokenType::Else)) {
            elsePart = block();
        }
        return new If(cond, then, elsePart);
    }

    if (match(TokenType::While)) {
        auto cond = expression();
        auto inside = block();
        return new While(cond, inside);
    }

    if (match(TokenType::Return)) {
        return new ReturnStatement(expressionStatement());
    }

    return expressionStatement();
}

AstNode* Parser::file() {
    auto file = new File();
    while (!check(TokenType::Eof)) {
        file->addStatement(declaration());
    }

    return file;
}
