#pragma once

#include "astnode.hpp"
#include "exceptions.hpp"

#include <variant>

struct Type;
using TypePtr = std::shared_ptr<Type>;

enum class PrimitiveType {
    String,
    I32,
    F64,
    Bool,
    Unit,
};

struct FunctionType {
    std::vector<TypePtr> params;
    TypePtr returnType;
};

struct ArrayType {
    TypePtr elementType;
};

struct Type {
    using AnyType = std::variant<PrimitiveType, FunctionType, ArrayType>;
    AnyType value;

    explicit Type(AnyType val) :
        value(std::move(val)) {}

    static TypePtr from(AnyType type) {
        return std::make_shared<Type>(type);
    }

    static TypePtr array(TypePtr elementType) {
        return from(ArrayType {std::move(elementType)});
    }

    static TypePtr function(std::vector<TypePtr> params, TypePtr returnType) {
        return from(FunctionType {std::move(params), std::move(returnType)});
    }
};

class TypeContext : public std::enable_shared_from_this<TypeContext> {
    std::unordered_map<std::string, TypePtr> scope;
    std::shared_ptr<TypeContext> parent;

public:
    explicit TypeContext(std::shared_ptr<TypeContext> ctx) :
        parent(ctx) {}

    static std::shared_ptr<TypeContext> create(
        std::shared_ptr<TypeContext> parent
    ) {
        return std::make_shared<TypeContext>(std::move(parent));
    }

    void define(std::string name, TypePtr type) {
        scope[name] = type;
    }

    TypePtr lookup(std::string name) const {
        auto ctx = shared_from_this();

        while (ctx) {
            if (auto res = ctx->scope.find(name); res != ctx->scope.end()) {
                return res->second;
            }
            ctx = ctx->parent;
        }

        throw std::runtime_error(std::format(
            "ReferenceError: Unknown type for identifier '{}'", name
        ));
    }

    std::shared_ptr<TypeContext> enterScope() {
        return create(shared_from_this());
    }

    std::shared_ptr<TypeContext> exitScope() {
        return parent;
    }
};

struct TypeChecker {
    std::shared_ptr<TypeContext> builtins;
    std::shared_ptr<TypeContext> ctx;

    TypePtr stringType = Type::from(PrimitiveType::String);
    TypePtr i32Type = Type::from(PrimitiveType::I32);
    TypePtr f64Type = Type::from(PrimitiveType::F64);
    TypePtr boolType = Type::from(PrimitiveType::Bool);
    TypePtr unitType = Type::from(PrimitiveType::Unit);

    SourceLocation currentLoc {1, 1};

    explicit TypeChecker() :
        builtins(TypeContext::create(nullptr)),
        ctx(builtins) {
        builtins->define("string", stringType);
        builtins->define("i32", i32Type);
        builtins->define("f64", f64Type);
        builtins->define("bool", boolType);
        builtins->define("unit", unitType);
    }

    bool isSameType(const TypePtr& a, const TypePtr& b) {
        if (a == b) {
            return true;
        }

        if (!a || !b || a->value.index() != b->value.index()) {
            return false;
        }

        if (auto pA = std::get_if<PrimitiveType>(&a->value)) {
            return *pA == std::get<PrimitiveType>(b->value);
        }

        if (auto arrA = std::get_if<ArrayType>(&a->value)) {
            return isSameType(arrA->elementType,
                              std::get<ArrayType>(b->value).elementType);
        }
        if (auto fnA = std::get_if<FunctionType>(&a->value)) {
            auto fnB = std::get<FunctionType>(b->value);
            if (fnA->params.size()
                != fnB.params.size()
                || !isSameType(fnA->returnType, fnB.returnType)) {
                return false;
            }

            for (size_t i = 0; i < fnA->params.size(); ++i) {
                if (!isSameType(fnA->params[i], fnB.params[i])) {
                    return false;
                }
            }
            return true;
        }

        return false;
    }

    template <typename T>
    TypePtr check(const std::unique_ptr<T>& node) {
        SourceLocation previousLoc = currentLoc;
        currentLoc = node->loc;
        TypePtr result = std::visit(*this, node->value);
        currentLoc = previousLoc;
        return result;
    }

    template <typename T>
    TypePtr check(const T& node) {
        SourceLocation previousLoc = currentLoc;
        currentLoc = node.loc;
        TypePtr result = std::visit(*this, node.value);
        currentLoc = previousLoc;
        return result;
    }

    [[noreturn]]
    void TypeError(std::string message) {
        error("TypeError", message);
    }

    [[noreturn]]
    void error(std::string errorType, std::string message) {
        throw std::runtime_error(std::format("{}: {} \n\t  at {}:{}",
                                             errorType,
                                             message,
                                             currentLoc.lineno,
                                             currentLoc.colno));
    }

    TypePtr operator()(const std::monostate&) {
        return unitType;
    }

    TypePtr operator()(const ExpressionPtr& node) {
        return check(node);
    }

    TypePtr operator()(const StatementPtr& node) {
        return check(node);
    }

    TypePtr operator()(double) {
        return f64Type;
    }

    TypePtr operator()(uint32_t) {
        return i32Type;
    }

    TypePtr operator()(bool) {
        return boolType;
    }

    TypePtr operator()(const StringLiteralNode&) {
        return stringType;
    }

    TypePtr operator()(const IdentifierNode& node) {
        return ctx->lookup(node.value);
    }

    TypePtr operator()(const CallNode& node) {
        auto lhs = check(node.name);
        if (auto functionType = std::get_if<FunctionType>(&lhs->value)) {
            if (functionType->params.size() != node.args.size()) {
                TypeError("incorrect number of arguments for function call");
            }

            for (size_t i = 0; i < node.args.size(); ++i) {
                auto&& arg = node.args[i];

                auto expected = functionType->params[i];
                auto actual = check(arg);

                if (!isSameType(expected, actual)) {
                    TypeError("invalid parameter type");
                }
            }
            return functionType->returnType;
        }

        TypeError("attempting to call a non-function");
    }

    TypePtr operator()(const UnaryNode& node) {
        return check(node.child);
    }

    TypePtr operator()(const BinaryNode& node) {
        auto lhsType = check(node.lhs);
        auto rhsType = check(node.rhs);
        // TODO: unify the types and return that. For now require the exact
        //       same types

        if (!isSameType(lhsType, rhsType)) {
            TypeError("invalid binary operation");
        }
        return lhsType;
    }

    // Statements

    TypePtr operator()(const ExpressionStatementNode& node) {
        check(node.expr);
        return unitType;
    }

    TypePtr operator()(const BlockNode& node) {
        ctx = ctx->enterScope();
        TypePtr lastStatement = unitType;

        for (auto&& statement : node.nodes) {
            lastStatement = check(statement);
        }

        ctx = ctx->exitScope();
        return lastStatement;
    }

    TypePtr operator()(const AssignmentNode& node) {
        auto lhsType = check(node.lhs);
        auto rhsType = check(node.rhs);
        if (!isSameType(lhsType, rhsType)) {
            TypeError("assignment operator has invalid types");
        }
        return unitType;
    }

    TypePtr operator()(const WhileNode& node) {
        auto conditionType = check(node.condition);
        if (!isSameType(conditionType, boolType)) {
            TypeError("while condition must be a boolean");
        }

        (*this)(node.body);

        return unitType;
    }

    TypePtr operator()(const ReturnNode& node) {
        check(node.expression);
        return unitType;
    }

    TypePtr operator()(const IfNode& node) {
        auto conditionType = check(node.condition);
        if (!isSameType(conditionType, boolType)) {
            TypeError("if condition must be a boolean");
        }

        auto thenType = (*this)(node.thenPart);
        // raw visit because we don't have a ptr
        auto elseType = std::visit(*this, node.elsePart);

        if (!isSameType(thenType, elseType)) {
            TypeError("if body and else body must match");
        }

        return thenType;
    }

    TypePtr operator()(const VarDeclNode& node) {
        auto name = node.name.value;

        bool hasType = !std::holds_alternative<std::monostate>(node.type);
        bool hasRhs = !std::holds_alternative<std::monostate>(node.rhs);

        // var foo;
        if (!hasType && !hasRhs) {
            TypeError("variable declaration must have a type annotation or "
                      "initial value");
        }

        // var foo: i32;
        if (hasType && !hasRhs) {
            auto declaredType = check(std::get<TypeNode>(node.type));
            ctx->define(name, declaredType);
            return unitType;
        }

        // var foo = ...;
        if (!hasType && hasRhs) {
            auto rhsType = check(std::get<ExpressionPtr>(node.rhs));
            ctx->define(name, rhsType);
            return unitType;
        }

        // var foo: i32 = ...;
        auto declaredType = check(std::get<TypeNode>(node.type));
        auto rhsType = check(std::get<ExpressionPtr>(node.rhs));

        if (!isSameType(declaredType, rhsType)) {
            TypeError("declared type and right-side type do not match");
        }

        ctx->define(name, declaredType);
        return unitType;
    }

    TypePtr operator()(const FunctionNode& node) {
        auto name = node.name.value;

        std::vector<TypePtr> paramTypes;
        paramTypes.reserve(node.params.size());

        for (auto&& param : node.params) {
            auto paramType = check(param.type);
            paramTypes.push_back(paramType);
        }
        auto returnType = check(node.returnType);

        auto functionType = Type::function(paramTypes, returnType);
        ctx->define(name, functionType);

        ctx = ctx->enterScope();
        // Extra virtual block, just for ease of the current API
        // Ideally would add the params directly into the new block scope
        // to avoid looping over params again

        ctx->define(name, functionType);

        for (size_t i = 0; i < node.params.size(); ++i) {
            ctx->define(node.params[i].name.value, paramTypes[i]);
        }

        (*this)(node.body);

        ctx = ctx->exitScope();

        return unitType;
    }

    TypePtr operator()(const FileNode& node) {
        for (auto&& declaration : node.statements) {
            check(declaration);
        }
        return unitType;
    }

    // Types

    TypePtr operator()(const TypeNode& node) {
        return check(node);
    }

    TypePtr operator()(const NamedTypeNode& node) {
        return builtins->lookup(node.name.value);
    }

    TypePtr operator()(const ArrayTypeNode& node) {
        return Type::array(check(node.elementType));
    }
};
