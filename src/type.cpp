#include "../include/type.hpp"

void TypeContext::define(std::string name, TypePtr type) {
    scope[name] = type;
}

TypePtr TypeContext::lookup(std::string name) const {
    auto ctx = shared_from_this();

    while (ctx) {
        if (auto res = ctx->scope.find(name); res != ctx->scope.end()) {
            return res->second;
        }
        ctx = ctx->parent;
    }

    TypeError("could not find name " + name, {});
}
