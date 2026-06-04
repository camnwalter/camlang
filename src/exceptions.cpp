#include "../include/exceptions.hpp"

#include <iostream>
#include <print>

void SyntaxError(std::string msg, uint32_t line, uint32_t col) {
    std::println(
        std::cerr, "SyntaxError: {}\n\t at line {}, col {}", msg, line, col
    );
    std::abort();
}
