#pragma once

#include "exceptions.hpp"
#include "token.hpp"

#include <cstring>
#include <format>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

class Lexer {
private:
    const std::string input;
    size_t index;
    size_t len;
    std::vector<Token*> tokens;
    uint32_t lineno;
    uint32_t colno;

private:
    void newline() {
        colno = 1;
        lineno++;
    }

    void whitespace() {
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

    void comment() {
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

    bool isAlpha(char c) {
        return isalpha(c) || c == '_';
    }

    char peek() {
        if (index >= len) {
            return 0;
        }

        return input[index];
    }

    void next() {
        index++;
        colno++;
    }

    char consume() {
        char c = peek();
        next();
        return c;
    }

    void number() {
        size_t idiff;
        size_t fdiff;
        auto sub = input.substr(index);

        int ival = std::stoi(sub, &idiff);
        double fval = std::stod(sub, &fdiff);

        // 1 vs 1.0
        if (fdiff > idiff) {
            // we have a float
            tokens.push_back(
                new Token(TokenType::FNumber, fval, lineno, colno)
            );

            index += fdiff;
            colno += fdiff;
        } else {
            tokens.push_back(new Token(
                TokenType::INumber, static_cast<uint32_t>(ival), lineno, colno
            ));

            index += idiff;
            colno += idiff;
        }
    }

    void identifier() {
        uint32_t startCol = colno;
        std::stringstream ss;
        while (isAlpha(peek()) || isdigit(peek())) {
            ss << consume();
        }

        auto str = ss.str();
        if (TokenTypes.containsRight(str)) {
            auto tt = TokenTypes.getRight(str);
            tokens.push_back(new Token(tt, str, lineno, startCol));
        } else {
            tokens.push_back(
                new Token(TokenType::Identifier, str, lineno, startCol)
            );
        }
    }

    void string() {
        uint32_t startLine = lineno;
        uint32_t startCol = colno;
        std::stringstream ss;
        ss << consume(); // opening "

        while (peek() != '"') {
            if (peek() == '\\') {
                /*
                TokenType::TODO: Add escaped characters:
                In the future, add octals, hex, unicode
                */
                next();

                // Can make this a lookup table
                switch (peek()) {
                    case '0':
                        ss << '\0';
                        break;
                    case 'n':
                        ss << '\n';
                        break;
                    case 'r':
                        ss << '\r';
                        break;
                    case 'v':
                        ss << '\v';
                        break;
                    case 't':
                        ss << '\t';
                        break;
                    case 'a':
                        ss << '\a';
                        break;
                    case 'b':
                        ss << '\b';
                        break;
                    case 'f':
                        ss << '\f';
                        break;
                    default:
                        // this works for \\ and \". If we don't find any
                        // special escape then just ignore the backslash
                        ss << consume();
                        break;
                }
            } else {
                if (peek() == '\n') {
                    // we have an unclosed quotation
                    SyntaxError("unclosed \"", lineno, startCol);
                }
                ss << consume();
            }
        }

        next(); // closing "

        tokens.push_back(
            new Token(TokenType::String, ss.str(), startLine, startCol)
        );
    }

    void special() {
        TokenType tt;
        std::string lexeme;

        uint32_t startCol = colno;

        char c = consume();
        switch (c) {
            case '+':
                if (peek() == '=') {
                    next();
                    tt = TokenType::PlusEq;
                    lexeme = "+=";
                } else {
                    tt = TokenType::Plus;
                    lexeme = "+";
                }
                break;
            case '-':
                if (peek() == '=') {
                    next();
                    tt = TokenType::MinusEq;
                    lexeme = "-=";
                } else {
                    tt = TokenType::Minus;
                    lexeme = "-";
                }
                break;
            case '*':
                if (peek() == '=') {
                    next();
                    tt = TokenType::StarEq;
                    lexeme = "*=";
                } else {
                    tt = TokenType::Star;
                    lexeme = "*";
                }
                break;
            case '/':
                if (peek() == '=') {
                    next();
                    tt = TokenType::SlashEq;
                    lexeme = "/=";
                } else {
                    tt = TokenType::Slash;
                    lexeme = "/";
                }
                break;
            case '^':
                tt = TokenType::Caret;
                lexeme = "^";
                break;
            case '~':
                tt = TokenType::BitNot;
                lexeme = "~";
                break;
            case '&':
                tt = TokenType::BitAnd;
                lexeme = "&";
                break;
            case '|':
                tt = TokenType::BitOr;
                lexeme = "|";
                break;
            case '!':
                if (peek() == '=') {
                    next();
                    tt = TokenType::Ne;
                    lexeme = "!=";
                } else {
                    tt = TokenType::Not;
                    lexeme = "!";
                }
                break;
            case '(':
                tt = TokenType::LParen;
                lexeme = "(";
                break;
            case ')':
                tt = TokenType::RParen;
                lexeme = ")";
                break;
            case '{':
                tt = TokenType::LBrace;
                lexeme = "{";
                break;
            case '}':
                tt = TokenType::RBrace;
                lexeme = "}";
                break;
            case '<':
                if (peek() == '=') {
                    next();
                    tt = TokenType::LE;
                    lexeme = "<=";
                } else if (peek() == '<') {
                    next();
                    tt = TokenType::LShift;
                    lexeme = "<<";
                } else {
                    tt = TokenType::LT;
                    lexeme = "<";
                }
                break;
            case '>':
                if (peek() == '=') {
                    next();
                    tt = TokenType::GE;
                    lexeme = ">=";
                } else if (peek() == '>') {
                    next();
                    tt = TokenType::RShift;
                    lexeme = ">>";
                } else {
                    tt = TokenType::GT;
                    lexeme = ">";
                }
                break;
            case '=':
                if (peek() == '=') {
                    next();
                    tt = TokenType::EqEq;
                    lexeme = "==";
                } else {
                    tt = TokenType::Eq;
                    lexeme = "=";
                }
                break;
            case ';':
                tt = TokenType::Semi;
                lexeme = ";";
                break;
            case ',':
                tt = TokenType::Comma;
                lexeme = ",";
                break;
            default:
                std::println(std::cerr, "Unknown token type, got {}", c);
                std::abort();
                break;
        }

        tokens.push_back(new Token(tt, lexeme, lineno, startCol));
    }

    void gettoken() {
        whitespace();

        char c = peek();
        if (isdigit(c)) {
            number();
        } else if (isAlpha(c)) {
            identifier();
        } else if (c == '"') {
            string();
        } else {
            special();
        }
    }

public:
    Lexer(std::string _input) :
        input(_input), index(0), len(_input.length()), lineno(1), colno(1) {}

    std::vector<Token*> lex() {
        if (index >= len) {
            // this can only happen if we try to call lex() more than once, so
            // we can just noop
            return tokens;
        }

        while (index < len) {
            gettoken();
        }

        tokens.push_back(new Token(TokenType::Eof, "", lineno, colno));

        std::cout << "Finished lexing" << std::endl;

        return tokens;
    }
};
