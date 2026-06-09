#include "../include/exceptions.hpp"

#include <iostream>
#include <print>

void SyntaxError(std::string msg, size_t line, size_t col) {
    std::println(
        std::cerr, "SyntaxError: {}\n\t at line {}, col {}", msg, line, col
    );
    std::abort();
}
