// #pragma once

// #include "astnode.hpp"

// #include <variant>

// struct StringType {};

// struct I32Type {};

// struct F64Type {};

// struct BoolType {};

// struct VoidType {};

// using Type = std::variant<StringType, I32Type, F64Type, BoolType, VoidType>;

// struct TypeContext {};

// // Type checks and infers missing types
// struct TypeChecker {
//     void operator()(const std::monostate&) {}

//     void operator()(const ExpressionPtr& node) {
//         std::visit(*this, node->value);
//     }

//     void operator()(const StatementPtr& node) {
//         std::visit(*this, node->value);
//     }

//     void operator()(double node) {
//         std::cout << node;
//     }

//     void operator()(uint32_t node) {
//         std::cout << node;
//     }

//     void operator()(bool node) {
//         std::cout << node;
//     }

//     void operator()(const StringNode& node) {
//         std::cout << "\"" << node.value << "\"";
//     }

//     void operator()(const IdentifierNode& node) {
//         std::cout << "Identifier(" << node.value << ")";
//     }

//     void operator()(const CallNode& node) {
//         std::cout << "Call(";
//         std::visit(*this, node.name->value);
//         std::cout << ", args: (";

//         auto sep = "";
//         for (auto&& arg : node.args) {
//             std::cout << sep;
//             std::visit(*this, arg->value);
//             sep = ", ";
//         }

//         std::cout << "))";
//     }

//     void operator()(const UnaryNode& node) {
//         std::cout << "Unary(op: " << typeToString[node.op] << ", child: ";
//         std::visit(*this, node.child->value);
//         std::cout << ")";
//     }

//     void operator()(const BinaryNode& node) {
//         std::cout << "Binary(op: " << typeToString[node.op] << ", lhs: ";
//         std::visit(*this, node.lhs->value);
//         std::cout << ", rhs: ";
//         std::visit(*this, node.rhs->value);
//         std::cout << ")";
//     }

//     // Statements

//     void operator()(const ExpressionStatementNode& node) {
//         std::visit(*this, node.expr->value);
//     }

//     void operator()(const BlockNode& node) {
//         std::cout << "Block(";
//         indent += 2;
//         for (auto&& statement : node.nodes) {
//             std::cout << std::endl;

//             for (size_t i = 0; i < indent; i++) {
//                 std::cout << " ";
//             }

//             std::visit(*this, statement->value);
//         }

//         std::cout << std::endl;
//         indent -= 2;
//         for (size_t i = 0; i < indent; i++) {
//             std::cout << " ";
//         }
//         std::cout << ")";
//     }

//     void operator()(const AssignmentNode& node) {
//         std::cout << "Assign(op: " << typeToString[node.op] << ", lhs: ";
//         std::visit(*this, node.lhs->value);
//         std::cout << ", rhs: ";
//         std::visit(*this, node.rhs->value);
//         std::cout << ")";
//     }

//     void operator()(const WhileNode& node) {
//         std::cout << "While(condition: ";
//         std::visit(*this, node.condition->value);
//         std::cout << ", body: ";
//         (*this)(node.body);
//         std::cout << ")";
//     }

//     void operator()(const ReturnNode& node) {
//         std::cout << "Return(expression: ";
//         std::visit(*this, node.expression->value);
//         std::cout << ")";
//     }

//     void operator()(const IfNode& node) {
//         std::cout << "If(condition: ";
//         std::visit(*this, node.condition->value);
//         std::cout << ", then: ";
//         (*this)(node.thenPart);
//         std::cout << ", else: ";
//         std::visit(*this, node.elsePart);
//         std::cout << ")";
//     }

//     void operator()(const VarDeclNode& node) {
//         std::cout << "VarDeclaration(type: ";
//         std::cout << (node.isConst ? "const" : "var");
//         std::cout << ", name: ";
//         (*this)(node.name);
//         std::cout << ", type: ";
//         std::visit(*this, node.type);
//         std::cout << ", value: ";
//         std::visit(*this, node.rhs);
//         std::cout << ")";
//     }

//     void operator()(const FunctionNode& node) {
//         std::cout << "Function(name: ";
//         (*this)(node.name);
//         std::cout << ", params: (";

//         auto sep = "";
//         for (auto&& pair : node.params) {
//             std::cout << sep;
//             (*this)(pair.first);
//             std::cout << ": ";
//             (*this)(pair.second);
//             sep = ", ";
//         }
//         std::cout << "), returnType: ";
//         (*this)(node.returnType);
//         std::cout << ", body: ";
//         (*this)(node.body);
//         std::cout << ")";
//     }

//     // File

//     void operator()(const File& node) {
//         for (auto&& declaration : node.statements) {
//             std::visit(*this, declaration->value);
//             std::cout << std::endl;
//         }
//     }
// };
