#pragma once

#include "../include/astnode.hpp"

#include <string>

[[noreturn]]
void SyntaxError(std::string msg, SourceLocation loc);
