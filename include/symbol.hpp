#pragma once

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

struct Type {};

struct PrimitiveType : Type {
    std::string name;
};

struct FunctionType : Type {
    std::vector<std::shared_ptr<Type>> paramTypes;
    std::shared_ptr<Type> returnType;
};

enum class SymbolKind {
    Var,
    Const,
    Function,
    Type,
};

struct Symbol {
    std::string name;
    SymbolKind kind;
    std::shared_ptr<Type> datatype;

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
    std::unordered_map<std::string_view, std::shared_ptr<Symbol>> env;
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

    void createBuiltin(std::string_view name) {
        auto globals = root->env;
        auto type = std::make_shared<PrimitiveType>(name);

        globals[name] = std::make_shared<Symbol>(name, SymbolKind::Type, type);
    }

    void initializeGlobals() {
        createBuiltin("bool");
        createBuiltin("f64");
        createBuiltin("i32");
        createBuiltin("string");
        createBuiltin("void");
    }

public:
    SymbolTable() :
        root(std::make_unique<Scope>()),
        currentScope(root.get()) {
        initializeGlobals();

        // We go into a global scope now
        enterScope();
    }

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

    // TODO: overhaul the api to make sense
    bool insert(std::string_view name, std::shared_ptr<Symbol> sym) {
        if (currentScope->env.contains(name)) {
            // redefinition, should error
            return false;
        }

        currentScope->env[name] = sym;
        return true;
    }

    std::optional<std::shared_ptr<Symbol>> lookup(std::string_view name) {
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
