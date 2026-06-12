#pragma once

#include "type.hpp"

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

enum class SymbolKind {
    Var,
    Const,
    Function,
    Type,
};

struct Symbol {
    SymbolKind kind;
    const Type* datatype;

    friend std::ostream& operator<<(std::ostream& os, const Symbol& sym) {
        static std::string kinds[] = {"Var", "Const", "Function", "Type"};

        os
            << "Kind(SymbolKind::"
            << kinds[static_cast<int>(sym.kind)]
            << ") Datatype("
            << sym.datatype
            << ")";

        return os;
    }
};

struct Scope {
    Scope* parent {};
    std::unordered_map<std::string_view, Symbol> env;
    std::vector<std::unique_ptr<Scope>> children;

    friend std::ostream& operator<<(std::ostream& os, const Scope& scope) {
        static size_t indent = 0;
        static size_t scopeNumber = 0;

        for (size_t i = 0; i < indent; i++) {
            os << " ";
        }

        os << "Scope " << scopeNumber << ":" << std::endl;
        for (auto&& pair : scope.env) {
            for (size_t i = 0; i < indent; i++) {
                os << " ";
            }

            os
                << "Symbol("
                << pair.first
                << ": "
                << pair.second
                << ")"
                << std::endl;
        }

        os << std::endl;

        indent += 2;
        scopeNumber++;
        for (auto&& child : scope.children) {
            os << *child;
        }
        indent -= 2;

        return os;
    }
};

class SymbolTable {
    std::unique_ptr<Scope> root;
    Scope* currentScope;

public:
    SymbolTable() :
        root(std::make_unique<Scope>()),
        currentScope(root.get()) {}

    void enterScope() {
        auto newScope = std::make_unique<Scope>();
        newScope->parent = currentScope;

        auto raw = newScope.get();
        currentScope->children.push_back(std::move(newScope));
        currentScope = raw;
    }

    void exitScope() {
        if (currentScope->parent) {
            currentScope = currentScope->parent;
        }
    }

    bool insert(std::string_view name, Symbol sym) {
        if (currentScope->env.contains(name)) {
            // redefinition, should error
            return false;
        }

        currentScope->env[name] = sym;
        return true;
    }

    void addMany(std::unordered_map<std::string_view, Symbol>&& additions) {
        currentScope->env.merge(std::move(additions));
    }

    std::optional<Symbol> lookup(std::string_view name) {
        auto scope = currentScope;
        while (scope) {
            if (auto res = scope->env.find(name); res != scope->env.end()) {
                return res->second;
            }

            scope = scope->parent;
        }

        return std::nullopt;
    }

    void print() {
        std::cout << "---Symbol Table---" << std::endl;
        std::cout << *root << std::endl;
    }
};
