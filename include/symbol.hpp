#pragma once

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

enum class DeclarationType {
    Var,
    Const,
    Function,
};

struct Symbol {
    DeclarationType declarationType;
};

struct Scope {
    Scope* parent;
    std::unordered_map<std::string, Symbol> env;
    std::vector<std::unique_ptr<Scope>> children;
};

class SymbolTable {
    std::unique_ptr<Scope> root;
    Scope* currentScope;

public:
    SymbolTable() {
        root = std::make_unique<Scope>();
        currentScope = root.get();
    }

    void enterScope() {
        auto newScope = std::make_unique<Scope>();
        newScope->parent = currentScope;

        auto raw = newScope.get();
        currentScope->children.push_back(std::move(newScope));
        currentScope = raw;
    }

    void exitScope() {
        if (currentScope->parent != nullptr) {
            currentScope = currentScope->parent;
        }
    }

    bool insert(const std::string& name, const Symbol& sym) {
        if (currentScope->env.contains(name)) {
            // redefinition, should error
            return false;
        }

        currentScope->env[name] = sym;
        return true;
    }

    void addMany(std::unordered_map<std::string, Symbol>&& additions) {
        currentScope->env.merge(additions);
    }

    std::optional<Symbol> lookup(const std::string& name) {
        auto scope = currentScope;
        while (scope != nullptr) {
            if (scope->env.contains(name)) {
                return scope->env[name];
            }

            scope = scope->parent;
        }

        return std::nullopt;
    }
};
