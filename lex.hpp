#pragma once

#include <string>
#include <vector>
#include <cstring>
#include <unordered_map>

#include "token.hpp"

class Lexer
{
private:
    std::string input;
    uint32_t index;
    size_t len;
    std::vector<Token *> tokens;

private:
    void whitespace()
    {
        bool looping = true;
        while (looping)
        {
            if (isspace(peek()))
            {
                next();
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
    }

    char consume()
    {
        char c = peek();
        next();
        return c;
    }

    void number()
    {
        int32_t current = 0;

        while (isdigit(peek()))
        {
            current *= 10;
            current += consume() - '0';
        }

        tokens.push_back(new Token(NUMBER, current));
    }

    void identifier()
    {
        std::stringstream ss;
        while (isAlpha(peek()) || isdigit(peek()))
        {
            ss << consume();
        }

        tokens.push_back(new Token(IDENTIFIER, ss.str()));
    }

    void string()
    {
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
                ss << consume();
            }
        }

        next(); // closing "

        tokens.push_back(new Token(STRING, ss.str()));
    }

    void special()
    {
        char c = consume();
        switch (c)
        {
        case '+':
            tokens.push_back(new Token(PLUS, "+"));
            break;
        case '-':
            tokens.push_back(new Token(MINUS, "-"));
            break;
        case '*':
            tokens.push_back(new Token(TIMES, "*"));
            break;
        case '/':
            tokens.push_back(new Token(DIVIDE, "/"));
            break;
        default:
            std::cerr << "found: " << c << std::endl;
            break;
        }
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
    Lexer(std::string _input) : input(_input)
    {
        index = 0;
        len = input.length();
    }

    std::vector<Token *> lex()
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

        tokens.push_back(new Token(eof, ""));

        std::cout << "Finished lexing" << std::endl;

        return tokens;
    }
};
