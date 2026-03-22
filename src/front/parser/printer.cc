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
    
    type_str += std::format("{}\"Type_Data\": \"{}\"", spaces, cerne::TypeDataNames.at(type->data)) + ",\n";
    type_str += std::format("{}\"IsConst\": {}", spaces, type->is_const) + ",\n";
    type_str += std::format("{}\"IsPointer\": {}", spaces, type->is_pointer) + ",\n";
    type_str += std::format("{}\"Type_Info\": 1", spaces) + ",\n";

    if(type->templated_type) {
        type_str += std::format("{}\"Templated_Type\": {}", spaces, type_print(type->templated_type.get(), identation + 4)) + ",\n";
    } else {
        type_str += std::format("{}\"Templated_Type\": null", spaces) + ",\n";
    }

    type_str += std::format("{}\"type\": \"Type\"", spaces) + "\n";
    type_str += spaces + "}";

    return type_str;
}

// sub node printers

std::string cerne::Leaf::print(size_t identation) {
    const auto& spaces = std::string(identation, ' ');
    std::string node_str = "{\n";

    node_str += std::format("{}\"value\": {}", spaces, value ? *value : "") + ",\n";
    node_str += std::format("{}\"is_number\": {}", spaces, is_number) + ",\n";
    node_str += std::format("{}\"type\": \"Leaf\"", spaces) + '\n';
    node_str += spaces + "}";

    return node_str;
}

std::string cerne::LiteralExpr::print(size_t identation) {
    const auto& spaces = std::string(identation, ' ');
    std::string node_str = "{\n";

    node_str += std::format("{}\"value\": {}", spaces, value ? *value : "") + ",\n";
    node_str += std::format("{}\"type\": \"LiteralExpr\"", spaces) + '\n';
    node_str += spaces + "}";

    return node_str;
}

std::string cerne::BinaryExpr::print(size_t identation) {
    const auto& spaces = std::string(identation, ' ');
    std::string node_str = "{\n";

    node_str += std::format("{}\"type\": \"BinaryExpr\",\n", spaces);
    node_str += std::format("{}\"LeftHandSide\": {}", spaces, lhs->print(identation+4)) + ",\n";
    node_str += std::format("{}\"RightHandSide\": {}", spaces,rhs->print(identation+4))  + ",\n";
    node_str += std::format("{}\"Operation_Type\": \"{}\"", spaces, TokenTypeNames.at(op))  + '\n';
    node_str += spaces + "}";

    return node_str;
}

std::string cerne::Parameter::print(size_t identation) {
    const auto& spaces = std::string(identation, ' ');
    std::string node_str = "{\n";

    node_str += std::format("{}\"Unpack\": {}", spaces, unpack) + ",\n";
    node_str += std::format("{}\"Symbol_Name\": \"{}\"", spaces, symb)  + ",\n";
    node_str += std::format("{}\"Type\": {}", spaces, type_print(ptype.get(), identation+4))  + '\n';
    node_str += spaces + "}";

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
    node_str += spaces + "}";

    return node_str;
}

std::string cerne::FunNode::print(size_t identation) {
    const auto& spaces = std::string(identation, ' ');
    std::string node_str = "{\n";

    node_str += std::format("{}\"Parameters\": [\n", spaces);
    for(size_t i = 0; i < parameters.size(); i++) {
        const auto& parameter = parameters[i];
        node_str += parameter->print(identation+4);
    }
    node_str += spaces + "],\n";
    node_str += spaces + std::format("{}\"Scope\": {}", spaces, body->print(identation+4)) + ",\n";
    node_str += spaces + std::format("{}\"Return_Type\": {}", spaces, type_print(return_type.get(), identation+4)) + ",\n";
    node_str += spaces + std::format("{}\"Name\": \"{}\"", spaces, name) + '\n';
    node_str += spaces + "}";

    return node_str;
}

/* --- Node print methods End --- */