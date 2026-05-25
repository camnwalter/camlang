#pragma once

#include <exception>
#include <sstream>

std::runtime_error SyntaxError(std::string msg, uint32_t line, uint32_t col)
{
    std::stringstream ss;
    ss << "SyntaxError: ";
    ss << msg;
    ss << "\n\t";
    ss << " at line " << line << ", col " << col << std::endl;
    throw std::runtime_error(ss.str());
}
