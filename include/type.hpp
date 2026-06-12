#pragma once

#include <memory>
#include <vector>

enum class BasicType {
    Void,
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

    friend std::ostream& operator<<(std::ostream& os, const Type& type) {
        return type.print(os);
    }

    virtual std::ostream& print(std::ostream& os) const {
        static std::string types[]
            = {"Void", "Boolean", "Int32", "Float64", "String", "Function"};

        os
            << this
            << " Kind:"
            << types[static_cast<int>(kind)]
            << ", Name: "
            << name;

        return os;
    }
};

struct FunctionType : public Type {
    const Type* returnType;
    std::vector<const Type*> parameterTypes;

    FunctionType() :
        Type(BasicType::Function, "") {}

    std::ostream& print(std::ostream& os) const override {
        os
            << this
            << " Kind: Function, Return Type: "
            << returnType
            << " Parameters: (";
        for (auto&& i : parameterTypes) {
            os << i << " ";
        }
        os << ")";

        return os;
    }
};

class TypeContext {
    using Key = const std::pair<const Type*, std::vector<const Type*>>;

    // Taken from https://stackoverflow.com/a/72073933
    struct Hash {
        std::size_t operator()(Key& key) const {
            size_t h = std::hash<const Type*> {}(key.first);
            for (auto x : key.second) {
                h ^= std::hash<const Type*> {}(x)
                   + 0x9E3779B9
                   + (h << 6)
                   + (h >> 2);
            }
            return h;
        }
    };

    std::vector<std::unique_ptr<Type>> types;
    std::unordered_map<std::string_view, const Type*> namedTypes;
    std::unordered_map<std::pair<const Type*, std::vector<const Type*>>,
                       const FunctionType*,
                       Hash>
        functions;

    void makeBuiltin(BasicType basic, std::string_view name) {
        auto type = std::make_unique<Type>(basic, name);
        namedTypes[name] = type.get();
        types.push_back(std::move(type));
    }

public:
    TypeContext() {
        makeBuiltin(BasicType::Void, "void");
        makeBuiltin(BasicType::Int32, "i32");
        makeBuiltin(BasicType::Float64, "f64");
        makeBuiltin(BasicType::String, "string");
        makeBuiltin(BasicType::Boolean, "bool");
    }

    const Type* get(std::string_view name) {
        if (auto res = namedTypes.find(name); res != namedTypes.end()) {
            return res->second;
        }

        return nullptr;
    }

    // gets existing type, or inserts an empty type that will be updated later
    const Type* getOrInsertDummy(std::string_view name) {
        return namedTypes[name];
    }

    const FunctionType* createFunctionType(const Type* returnType,
                                           std::vector<const Type*> params) {
        auto key = std::make_pair(returnType, params);
        if (auto res = functions.find(key); res != functions.end()) {
            return res->second;
        }

        auto dummy = std::make_unique<FunctionType>();
        dummy->parameterTypes = params;
        dummy->returnType = returnType;
        // we don't care about the name for a functionType

        auto raw = dummy.get();
        functions[key] = raw;
        types.push_back(std::move(dummy));
        return raw;
    }

    const FunctionType* createFunction(std::string_view name,
                                       const Type* returnType,
                                       std::vector<const Type*> params) {
        auto type = createFunctionType(returnType, params);
        namedTypes[name] = type;
        return type;
    }

    void print() {
        std::cout << "---Types---" << std::endl;
        for (auto&& type : namedTypes) {
            std::cout << type.first << " " << *type.second << std::endl;
        }
    }
};
