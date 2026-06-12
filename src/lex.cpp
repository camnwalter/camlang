#include "../include/lex.hpp"

#include "../include/exceptions.hpp"
#include "../include/token.hpp"

#include <charconv>
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
    while (!isAtEnd() && peek() != '\n') {
        next();
    }
    // consume \n
    next();
    newline();
}

void Lexer::number() {
    size_t startIndex = index;

    const char* start_ptr = input.data() + startIndex;
    const char* end_ptr = input.data() + input.size();

    size_t idiff = 0;
    size_t fdiff = 0;

    int ival = 0;
    double fval = 0.0;

    // Try parsing as an integer
    auto [iptr, iec] = std::from_chars(start_ptr, end_ptr, ival);
    if (iec == std::errc()) {
        idiff = iptr - start_ptr;
    }

    // Try parsing as a double
    auto [fptr, fec] = std::from_chars(start_ptr, end_ptr, fval);
    if (fec == std::errc()) {
        fdiff = fptr - start_ptr;
    }

    // 1 vs 1.0
    if (fdiff > idiff) {
        // we have a float
        tokens.push_back({.tokenType = TokenType::FNumber,
                          .lexeme = input.substr(startIndex, fdiff),
                          .literal = fval,
                          .line = line,
                          .column = column});

        index += fdiff;
        column += fdiff;
    } else {
        tokens.push_back({.tokenType = TokenType::INumber,
                          .lexeme = input.substr(startIndex, idiff),
                          .literal = static_cast<uint32_t>(ival),
                          .line = line,
                          .column = column});

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
    TokenType type = TokenType::Identifier;
    if (auto res = reservedKeywords.find(str); res != reservedKeywords.end()) {
        type = res->second;
    }

    tokens.push_back({.tokenType = type,
                      .lexeme = str,
                      .literal = {},
                      .line = line,
                      .column = startCol});
}

static const std::unordered_map<char, char> rawToEscaped = {
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
    next(); // opening "

    while (peek() != '"' && peek() != '\n' && !isAtEnd()) {
        if (peek() == '\\') {
            next();

            if (isAtEnd()) {
                break;
            }

            /*
            TODO: Add more escaped characters:
            In the future, add octals, hex, unicode
            */

            if (auto res = rawToEscaped.find(peek());
                res != rawToEscaped.end()) {
                ss << res->second;
                next();
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

    tokens.push_back({.tokenType = TokenType::String,
                      .lexeme = rawString,
                      .literal = ss.str(),
                      .line = startLine,
                      .column = startCol});
}

static const std::unordered_map<std::string, TokenType> twoCharOperators = {
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

static const std::unordered_map<char, TokenType> oneCharOperators = {
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
    {':', TokenType::Colon},
};

void Lexer::special() {
    TokenType tt = TokenType::Error;
    size_t startIndex = index;
    size_t startCol = column;

    char c = consume();
    char potentialNext = peek();

    std::string potentialWideOp = {c, potentialNext, '\0'};

    if (auto res = twoCharOperators.find(potentialWideOp);
        res != twoCharOperators.end()) {
        tt = res->second;
        next();
    } else if (auto res = oneCharOperators.find(c);
               res != oneCharOperators.end()) {
        tt = res->second;
    } else {
        // TODO: Better error checking
        std::println(std::cerr,
                     "Index: {}, Unknown token type, saw '{}'",
                     startIndex,
                     potentialWideOp);
        std::abort();
    }

    tokens.push_back({.tokenType = tt,
                      .lexeme = input.substr(startIndex, index - startIndex),
                      .literal = {},
                      .line = line,
                      .column = startCol});
}
