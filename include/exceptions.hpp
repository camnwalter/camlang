#pragma once

#include <cstdint>
#include <string>

[[noreturn]] void SyntaxError(std::string msg, uint32_t line, uint32_t col);