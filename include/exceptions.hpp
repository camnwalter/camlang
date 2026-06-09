#pragma once

#include <cstdint>
#include <string>

[[noreturn]] void SyntaxError(std::string msg, size_t line, size_t col);
