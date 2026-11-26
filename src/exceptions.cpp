#include "../include/exceptions.hpp"

#include <iostream>
#include <print>

void SyntaxError(std::string msg, SourceLocation loc) {
    std::println(std::cerr,
                 "SyntaxError: {}\n\t at line {}, col {}",
                 msg,
                 loc.lineno,
                 loc.colno);
    std::abort();
}

void TypeError(std::string msg, SourceLocation loc) {
    std::println(std::cerr,
                 "TypeError: {}\n\t at line {}, col {}",
                 msg,
                 loc.lineno,
                 loc.colno);
    std::abort();
}
