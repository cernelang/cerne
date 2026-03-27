/*
    Cerne Compiler - Subcomponent of the parser, responsible for printing the AST.

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#include "../include/parser.hpp"

/* --- Node print methods Begin --- */

// quick helpers

std::string type_path_to_json(cerne::TypePath typepath) {
    return ""; // temporary
}

cerne::JSON type_to_json(cerne::Type* type) {
    cerne::JSON json;
    json.properties["Type_Data"] = cerne::TypeDataNames.at(type->data);
    json.properties["IsConst"] = std::format("{}", type->is_const);
    json.properties["IsPointer"] = std::format("{}", type->is_pointer);
    json.properties["Type_Info"] = std::holds_alternative<cerne::Primitive>(type->typeinfo) ? std::format("{}", type->typeinfo.index()) : "1";

    if(type->templated_type) {
        json.properties["Templated_Type"] = type_to_json(type->templated_type.get());
    } else {
        json.properties["Templated_Type"] = "null";
    }

    json.properties["type"] = "Type";
    return json;
}

// sub node JSON converters

cerne::JSON cerne::Leaf::to_json() {
    cerne::JSON json;
    json.properties["value"] = value ? *value : "";
    json.properties["is_number"] = std::format("{}", is_number);
    json.properties["type"] = "Leaf";
    return json;
}

cerne::JSON cerne::LiteralExpr::to_json() {
    cerne::JSON json;
    json.properties["value"] = value ? *value : "";
    json.properties["type"] = "LiteralExpr";
    return json;
}

cerne::JSON cerne::BinaryExpr::to_json() {
    cerne::JSON json;
    json.properties["type"] = "BinaryExpr";
    json.properties["LeftHandSide"] = lhs->to_json();
    json.properties["RightHandSide"] = rhs->to_json();
    json.properties["Operation_Type"] = TokenTypeNames.at(op);
    return json;
}

cerne::JSON cerne::Parameter::to_json() {
    cerne::JSON json;
    json.properties["Unpack"] = std::format("{}", unpack);
    json.properties["Symbol_Name"] = std::string(symb);
    json.properties["Type"] = type_to_json(ptype.get());
    json.properties["type"] = "Parameter";
    return json;
}

cerne::JSON cerne::Scope::to_json() {
    cerne::JSON json;
    std::vector<cerne::JSON> nodes_json;
    for(size_t i = 0; i < body.size(); i++) {
        const auto& node = body[i];
        nodes_json.push_back(node->to_json());
    }
    json.properties["Nodes"] = JSONBuilder{}.convert_array(nodes_json);
    json.properties["type"] = "Scope";
    return json;
}

cerne::JSON cerne::FunNode::to_json() {
    cerne::JSON json;
    std::vector<cerne::JSON> params_json;
    for(size_t i = 0; i < parameters.size(); i++) {
        const auto& parameter = parameters[i];
        params_json.push_back(parameter->to_json());
    }
    json.properties["Parameters"] = JSONBuilder{}.convert_array(params_json);
    json.properties["Scope"] = body->to_json();
    json.properties["Return_Type"] = type_to_json(return_type.get());
    json.properties["Name"] = std::string(name);
    json.properties["type"] = "FunNode";
    return json;
}

cerne::JSON cerne::VarDecl::to_json() {
    cerne::JSON json;
    json.properties["Name"] = std::string(name);
    json.properties["Is_Const"] = std::format("{}", is_const);
    json.properties["Uninitialized"] = std::format("{}", uninitialized);
    json.properties["Type"] = type_to_json(type.get());
    json.properties["Value"] = value ? JSONBuilder{value->to_json()}.build() : "null";
    json.properties["type"] = "VarDecl";
    return json;
}

cerne::JSON cerne::ReturnStmt::to_json() {
    cerne::JSON json;
    std::vector<cerne::JSON> values_json;
    for(size_t i = 0; i < values.size(); i++) {
        const auto& value = values[i];
        values_json.push_back(value->to_json());
    }
    json.properties["Values"] = JSONBuilder{}.convert_array(values_json);
    json.properties["type"] = "ReturnStmt";
    return json;
}

/* --- Node print methods End --- */