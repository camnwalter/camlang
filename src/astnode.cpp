#include "../include/astnode.hpp"

SourceLocation tokenLocation(Token tok) {
    return {tok.line, tok.column};
}
