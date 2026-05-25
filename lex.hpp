#pragma once

#include <string>
#include <vector>
#include <cstring>
#include <unordered_map>

#include "token.hpp"
#include "exceptions.hpp"

class Lexer
{
private:
    const std::string input;
    size_t index;
    size_t len;
    std::vector<TokenPtr> tokens;
    uint32_t lineno;
    uint32_t colno;

private:
    void newline()
    {
        colno = 1;
        lineno++;
    }

    void whitespace()
    {
        bool looping = true;
        while (looping)
        {
            if (isspace(peek()))
            {
                if (peek() == '\n')
                {
                    next();
                    newline();
                }
                else
                {
                    next();
                }
            }
            else if (peek() == '#')
            {
                comment();
            }
            else
            {
                looping = false;
            }
        }
    }

    void comment()
    {
        // consume #
        next();

        // now skip rest of line
        while (peek() != '\n')
        {
            next();
        }
        // consume \n
        next();
        newline();
    }

    bool isAlpha(char c)
    {
        return isalpha(c) || c == '_';
    }

    char peek()
    {
        if (index >= len)
        {
            return 0;
        }

        return input[index];
    }

    void next()
    {
        index++;
        colno++;
    }

    char consume()
    {
        char c = peek();
        next();
        return c;
    }

    void number()
    {
        size_t diff;
        double value = std::stod(input.substr(index), &diff);

        index += diff;

        tokens.push_back(std::make_shared<Token>(NUMBER, value, lineno, colno));
        colno += diff;
    }

    void identifier()
    {
        uint32_t startCol = colno;
        std::stringstream ss;
        while (isAlpha(peek()) || isdigit(peek()))
        {
            ss << consume();
        }

        auto str = ss.str();
        if (textToTokenType().contains(str))
        {
            TokenType tt = textToTokenType()[str];
            tokens.push_back(std::make_shared<Token>(tt, str, lineno, startCol));
        }
        else
        {
            tokens.push_back(std::make_shared<Token>(IDENTIFIER, str, lineno, startCol));
        }
    }

    void string()
    {
        uint32_t startLine = lineno;
        uint32_t startCol = colno;
        std::stringstream ss;
        ss << consume(); // opening "

        while (peek() != '"')
        {
            if (peek() == '\\')
            {
                /*
                TODO: Add escaped characters:
                In the future, add octals, hex, unicode
                */
                next();

                // Can make this a lookup table
                switch (peek())
                {
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
                    // this works for \\ and \". If we don't find any special
                    // escape then just ignore the backslash
                    ss << consume();
                    break;
                }
            }
            else
            {
                if (peek() == '\n')
                {
                    // we have an unclosed quotation
                    throw SyntaxError("unclosed \"", lineno, startCol);
                }
                ss << consume();
            }
        }

        next(); // closing "

        tokens.push_back(std::make_shared<Token>(STRING, ss.str(), startLine, startCol));
    }

    void special()
    {
        TokenType tt;
        std::string lexeme;

        uint32_t startCol = colno;

        char c = consume();
        switch (c)
        {
        case '+':
            tt = PLUS;
            lexeme = "+";
            break;
        case '-':
            tt = MINUS;
            lexeme = "-";
            break;
        case '*':
            tt = STAR;
            lexeme = "*";
            break;
        case '/':
            tt = SLASH;
            lexeme = "/";
            break;
        case '^':
            tt = CARET;
            lexeme = "^";
            break;
        case '!':
            if (peek() == '=')
            {
                next();
                tt = NE;
                lexeme = "!=";
            }
            else
            {
                tt = NOT;
                lexeme = "!";
            }
            break;
        case '(':
            tt = LPAREN;
            lexeme = "(";
            break;
        case ')':
            tt = RPAREN;
            lexeme = ")";
            break;
        case '{':
            tt = LBRACE;
            lexeme = "{";
            break;
        case '}':
            tt = RBRACE;
            lexeme = "}";
            break;
        case '<':
            if (peek() == '=')
            {
                next();
                tt = LE;
                lexeme = "<=";
            }
            else
            {
                tt = LT;
                lexeme = "<";
            }
            break;
        case '>':
            if (peek() == '=')
            {
                next();
                tt = GE;
                lexeme = ">=";
            }
            else
            {
                tt = GT;
                lexeme = ">";
            }
            break;
        case '=':
            tt = EQ;
            lexeme = "=";
            break;
        case ';':
            tt = SEMICOLON;
            lexeme = ";";
            break;
        default:
            std::cerr << "found: " << c << std::endl;
            break;
        }

        tokens.push_back(std::make_shared<Token>(tt, lexeme, lineno, startCol));
    }

    void gettoken()
    {
        whitespace();

        char c = peek();
        if (isdigit(c))
        {
            number();
        }
        else if (isAlpha(c))
        {
            identifier();
        }
        else if (c == '"')
        {
            string();
        }
        else
        {
            special();
        }
    }

public:
    Lexer(std::string _input) : input(_input), index(0), len(_input.length()), lineno(1), colno(1)
    {
    }

    std::vector<TokenPtr> lex()
    {
        if (index >= len)
        {
            // this can only happen if we try to call lex() more than once, so
            // we can just noop
            return tokens;
        }

        while (index < len)
        {
            gettoken();
        }

        tokens.push_back(std::make_shared<Token>(eof, "", lineno, colno));

        std::cout << "Finished lexing" << std::endl;

        return tokens;
    }
};
