#pragma once

#include "astnode.hpp"

#include <memory>
#include <vector>

enum class BasicType {
    Unknown,
    Void,
    Unit,
    Boolean,
    Int32,
    Float64,
    String,
    Function,
};

struct Type {
    BasicType kind;
    std::string_view name; // can be empty

    Type(BasicType k, std::string_view n) :
        kind(k),
        name(n) {}

    virtual ~Type() = default;

    friend std::ostream& operator<<(std::ostream& os, Type& type) {
        return type.print(os);
    }

    virtual std::ostream& print(std::ostream& os) const {
        static std::string types[]
            = {"Void", "Boolean", "Int32", "Float64", "String", "Function"};

        os
            << "Type(this: "
            << this
            << " kind: BasicType::"
            << types[static_cast<int>(kind)]
            << ", name: "
            << name
            << ")";

        return os;
    }

    virtual Type* unwrap() {
        return this;
    }

    virtual const Type* unwrap() const {
        return this;
    }
};

struct FunctionType : public Type {
    Type* returnType;
    std::vector<Type*> parameterTypes;

    FunctionType(Type* ret, std::vector<Type*> params) :
        Type(BasicType::Function, ""),
        returnType(ret),
        parameterTypes(params) {}

    std::ostream& print(std::ostream& os) const override {
        os
            << "FunctionType(this: "
            << this
            << " kind: BasicType::Function, returnType: "
            << returnType->unwrap()
            << " parameterTypes: (";
        for (auto&& i : parameterTypes) {
            os << i->unwrap() << " ";
        }
        os << "))";

        return os;
    }
};

struct PlaceholderType : public Type {
    Type* internal = nullptr;

    PlaceholderType(std::string_view name) :
        Type(BasicType::Unknown, name) {}

    std::ostream& print(std::ostream& os) const override {
        if (internal != nullptr) {
            return internal->unwrap()->print(os);
        }

        os << "PlaceholderType(this: " << this << " , name: " << name << ")";

        return os;
    }

    Type* unwrap() override {
        return internal == nullptr ? this : internal->unwrap();
    }

    const Type* unwrap() const override {
        return internal == nullptr ? this : internal->unwrap();
    }
};

class TypeContext {
    using Key = const std::pair<Type*, std::vector<Type*>>;

    // Taken from https://stackoverflow.com/a/72073933
    struct Hash {
        std::size_t operator()(Key& key) const {
            size_t h = std::hash<Type*> {}(key.first);
            for (auto x : key.second) {
                h ^= std::hash<Type*> {}(x) + 0x9E3779B9 + (h << 6) + (h >> 2);
            }
            return h;
        }
    };

    std::vector<std::unique_ptr<Type>> types;
    std::unordered_map<std::string_view, Type*> namedTypes;
    std::
        unordered_map<std::pair<Type*, std::vector<Type*>>, FunctionType*, Hash>
            functions;
    std::unordered_map<std::string_view, Type*> functionToReturnTypes;
    size_t typeId {0};

    void makeBuiltin(BasicType basic, std::string_view name) {
        auto type = std::make_unique<Type>(basic, name);
        namedTypes[name] = type.get();
        types.push_back(std::move(type));
    }

public:
    TypeContext() {
        makeBuiltin(BasicType::Void, "void");
        makeBuiltin(BasicType::Unit, "unit");
        makeBuiltin(BasicType::Int32, "i32");
        makeBuiltin(BasicType::Float64, "f64");
        makeBuiltin(BasicType::String, "string");
        makeBuiltin(BasicType::Boolean, "bool");
    }

    Type* unify(Type* t1, Type*) {
        return t1;
    }

    // get or fail if the name isn't found
    Type* get(std::string_view name) {
        if (auto res = namedTypes.find(name); res != namedTypes.end()) {
            return res->second;
        }

        throw std::runtime_error("Unknown type name");
    }

    Type* getOrCreateFunctionReturnType(std::string_view name) {
        if (auto res = functionToReturnTypes.find(name);
            res != functionToReturnTypes.end()) {
            return res->second;
        }
        auto dummy = createDummy();
        functionToReturnTypes[name] = dummy;
        return dummy;
    }

    Type* getOrCreateDummy(std::string_view name) {
        if (auto res = namedTypes.find(name); res != namedTypes.end()) {
            return res->second;
        }

        auto type = createDummy();
        namedTypes[name] = type;
        return type;
    }

    Type* createDummy() {
        auto type
            = std::make_unique<PlaceholderType>("?" + std::to_string(typeId++));
        auto raw = type.get();
        types.push_back(std::move(type));
        return raw;
    }

    const Type* createFunction(std::string_view name,
                               Type* returnType,
                               std::vector<Type*> params) {
        auto key = std::make_pair(returnType, params);
        if (auto res = functions.find(key); res != functions.end()) {
            return res->second;
        }

        auto func = std::make_unique<FunctionType>(returnType, params);
        auto raw = func.get();

        functions[key] = raw;
        functionToReturnTypes[name] = returnType;
        namedTypes[name] = raw;
        types.push_back(std::move(func));
        return raw;
    }

    void print() {
        std::cout << "---Types---" << std::endl;
        for (auto&& type : namedTypes) {
            std::cout << *type.second << std::endl;
        }
    }
};
