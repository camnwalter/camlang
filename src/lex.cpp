#include "../include/lex.hpp"

#include "../include/exceptions.hpp"
#include "../include/token.hpp"

#include <sstream>

void Lexer::whitespace() {
    bool looping = true;
    while (looping) {
        if (isspace(peek())) {
            if (peek() == '\n') {
                next();
                newline();
            } else {
                next();
            }
        } else if (peek() == '#') {
            comment();
        } else {
            looping = false;
        }
    }
}

void Lexer::comment() {
    // consume #
    next();

    // now skip rest of line
    while (peek() != '\n') {
        next();
    }
    // consume \n
    next();
    newline();
}

void Lexer::number() {
    size_t idiff;
    size_t fdiff;
    auto sub = input.substr(index);

    int ival = std::stoi(sub, &idiff);
    double fval = std::stod(sub, &fdiff);

    // 1 vs 1.0
    if (fdiff > idiff) {
        // we have a float
        tokens.push_back(
            {TokenType::FNumber, sub.substr(0, fdiff), fval, line, column}
        );

        index += fdiff;
        column += fdiff;
    } else {
        tokens.push_back({TokenType::INumber,
                          sub.substr(0, idiff),
                          static_cast<uint32_t>(ival),
                          line,
                          column});

        index += idiff;
        column += idiff;
    }
}

void Lexer::identifier() {
    size_t startIndex = index;

    size_t startCol = column;
    while (isAlpha(peek()) || isdigit(peek())) {
        next();
    }

    auto str = input.substr(startIndex, index - startIndex);
    TokenType type;
    if (reservedKeywords.contains(str)) {
        type = reservedKeywords[str];
    } else {
        type = TokenType::Identifier;
    }

    tokens.push_back({type, str, {}, line, startCol});
}

static std::unordered_map<char, char> rawToEscaped = {
    {'0', '\0'},
    {'n', '\n'},
    {'r', '\r'},
    {'v', '\v'},
    {'t', '\t'},
    {'b', '\b'},
    {'f', '\f'},
};

void Lexer::string() {
    size_t startIndex = index;

    size_t startLine = line;
    size_t startCol = column;

    std::stringstream ss;
    ss << consume(); // opening "

    bool hasEscapes = false;
    while (peek() != '"' && peek() != '\n' && !isAtEnd()) {
        if (peek() == '\\') {
            next();

            if (isAtEnd()) {
                break;
            }

            hasEscapes = true;

            /*
            TODO: Add more escaped characters:
            In the future, add octals, hex, unicode
            */

            if (rawToEscaped.contains(peek())) {
                ss << rawToEscaped[consume()];
            } else {
                ss << consume();
            }
        } else {
            ss << consume();
        }
    }

    if (isAtEnd()) {
        SyntaxError("unclosed string", line, startCol);
    }

    next(); // closing "

    auto rawString = input.substr(startIndex, index - startIndex);
    auto noQuotes = input.substr(startIndex + 1, index - startIndex - 2);

    tokens.push_back({TokenType::String,
                      rawString,
                      hasEscapes ? ss.str() : noQuotes,
                      startLine,
                      startCol});
}

static std::unordered_map<std::string, TokenType> twoCharOperators = {
    {"+=", TokenType::PlusEq},
    {"-=", TokenType::MinusEq},
    {"*=", TokenType::StarEq},
    {"/=", TokenType::SlashEq},
    {"!=", TokenType::BangEq},
    {"==", TokenType::EqEq},
    {"<=", TokenType::LAngleEq},
    {">=", TokenType::RAngleEq},
    {"<<", TokenType::LShift},
    {">>", TokenType::RShift},
};

static std::unordered_map<char, TokenType> oneCharOperators = {
    {'+', TokenType::Plus},
    {'-', TokenType::Minus},
    {'*', TokenType::Star},
    {'/', TokenType::Slash},
    {'!', TokenType::Bang},
    {'<', TokenType::LAngle},
    {'>', TokenType::RAngle},
    {'=', TokenType::Eq},
    {'^', TokenType::Caret},
    {'~', TokenType::Tilde},
    {'&', TokenType::BitAnd},
    {'|', TokenType::BitOr},
    {'(', TokenType::LParen},
    {')', TokenType::RParen},
    {'{', TokenType::LBrace},
    {'}', TokenType::RBrace},
    {';', TokenType::Semi},
    {',', TokenType::Comma},
};

void Lexer::special() {
    TokenType tt;
    size_t startIndex = index;
    size_t startCol = column;

    char c = consume();
    char potentialNext = peek();

    char potentialWideOp[3] = {c, potentialNext, '\0'};

    if (twoCharOperators.contains(potentialWideOp)) {
        tt = twoCharOperators[potentialWideOp];
        next();
    } else if (oneCharOperators.contains(c)) {
        tt = oneCharOperators[c];
    } else {
        // TODO: Better error checking
        std::println(std::cerr,
                     "Index: {}, Unknown token type, saw '{}'",
                     startIndex,
                     potentialWideOp);
        std::abort();
    }

    tokens.push_back(
        {tt, input.substr(startIndex, index - startIndex), {}, line, startCol}
    );
}
