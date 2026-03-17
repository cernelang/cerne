/*
    Cerne Compiler - Subcomponent of the parser, responsible for printing the AST.

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#include "../include/parser.hpp"

/* --- Node print methods Begin --- */

// quick helper
std::string type_print(cerne::Type* type, size_t identation) {
    if(!type) return "{}";

    const auto& spaces = std::string(identation, ' ');
    std::string type_str = "{\n";
    
    type_str += spaces + std::format("\"Type_Data\": \"{}\"", cerne::TypeDataNames.at(type->data)) + ",\n";
    type_str += spaces + std::format("\"IsConst\": {}", type->is_const) + ",\n";
    type_str += spaces + std::format("\"IsPointer\": {}", type->is_pointer) + ",\n";
    type_str += spaces + std::format("\"Type_Info\": 1") + ",\n";

    if(type->templated_type) {
        type_str += spaces + std::format("\"Templated_Type\": {}", type_print(type->templated_type.get(), identation + 4)) + "\n";
    } else {
        type_str += spaces + "\"Templated_Type\": null\n";
    }

    type_str += spaces + "}\n";

    return type_str;
}

// sub node printers

std::string cerne::Leaf::print(size_t identation) {
    const auto& spaces = std::string(identation, ' ');
    std::string node_str = "{\n";

    node_str += spaces + std::format("\"value\": {}", value ? *value : "") + ",\n";
    node_str += spaces + std::format("\"is_number\": {}", is_number) + ",\n";
    node_str += spaces + "}\n";

    return node_str;
}

std::string cerne::LiteralExpr::print(size_t identation) {
    const auto& spaces = std::string(identation, ' ');
    std::string node_str = "{\n";

    node_str += spaces + std::format("\"value\": {}", value ? *value : "") + ",\n";
    node_str += spaces + "}\n";

    return node_str;
}

std::string cerne::BinaryExpr::print(size_t identation) {
    const auto& spaces = std::string(identation, ' ');
    std::string node_str = "{\n";

    node_str += spaces + std::format("\"LeftHandSide\": {}", lhs->print(identation+4)) + ",\n";
    node_str += spaces + std::format("\"RightHandSide\": {}", rhs->print(identation+4))  + ",\n";
    node_str += spaces + std::format("\"Operation_Type\": \"{}\"", TokenTypeNames.at(op))  + '\n';
    node_str += spaces + "}\n";

    return node_str;
}

std::string cerne::Parameter::print(size_t identation) {
    const auto& spaces = std::string(identation, ' ');
    std::string node_str = "{\n";

    node_str += spaces + std::format("\"Unpack\": {}", unpack) + ",\n";
    node_str += spaces + std::format("\"Symbol_Name\": \"{}\"", symb)  + ",\n";
    node_str += spaces + std::format("\"Type\": {}", type_print(ptype.get(), identation+4))  + '\n';
    node_str += spaces + "}\n";

    return node_str;
}

std::string cerne::Scope::print(size_t identation) {
    const auto& spaces = std::string(identation, ' ');
    std::string node_str = "{\n";

    node_str += spaces + "\"Nodes\": [\n";
    for(size_t i = 0; i < body.size(); i++) {
        const auto& node = body[i];
        node_str += node->print(identation+4);
    }
    node_str += spaces + "]\n";
    node_str += spaces + "}\n";

    return node_str;
}

std::string cerne::FunNode::print(size_t identation) {
    const auto& spaces = std::string(identation, ' ');
    std::string node_str = "{\n";

    node_str += spaces + std::format("\"Parameters\": [\n");
    for(size_t i = 0; i < parameters.size(); i++) {
        const auto& parameter = parameters[i];
        node_str += parameter->print(identation+4);
    }
    node_str += spaces + "],\n";
    node_str += spaces + std::format("\"Scope\": {}", body->print(identation+4)) + ",\n";
    node_str += spaces + std::format("\"Return_Type\": {}", type_print(return_type.get(), identation+4)) + ",\n";
    node_str += spaces + std::format("\"Name\": \"{}\"", name) + '\n';
    node_str += spaces + "}\n";

    return node_str;
}

/* --- Node print methods End --- */