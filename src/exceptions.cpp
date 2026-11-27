#include "../include/exceptions.hpp"

#include <iostream>
#include <print>

void SyntaxError(std::string msg, SourceLocation loc) {
    throw std::runtime_error(std::format(
        "SyntaxError: {}\n\t at line {}, col {}", msg, loc.lineno, loc.colno
    ));
}
