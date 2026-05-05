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

std::string type_path_to_json(const cerne::TypePath& typepath) {
    std::vector<cerne::JSON> elements_json;

    for(size_t i = 0; i < typepath.size(); i++) {
        const auto& element = typepath[i];

        cerne::JSON element_json;
        element_json.properties["name"] = std::string(element.name);
        element_json.properties["is_member"] = std::format("{}", element.is_member);
        elements_json.push_back(element_json);
    }

    return cerne::JSONBuilder{}.convert_array(elements_json);
}

cerne::JSON type_to_json(const cerne::Type* type) {
    cerne::JSON json;
    json.properties["type_data"] = cerne::TypeDataNames.at(type->data);
    json.properties["is_const"] = std::format("{}", type->is_const);
    json.properties["is_pointer"] = std::format("{}", type->is_pointer);
    json.properties["type_info"] = std::holds_alternative<cerne::Primitive>(type->typeinfo) ? std::format("{}", type->typeinfo.index()) : "1";

    if(type->templated_type) {
        json.properties["templated_type"] = type_to_json(type->templated_type.get());
    } else {
        json.properties["templated_type"] = "null";
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
    json.properties["left_hand_side"] = lhs->to_json();
    json.properties["right_hand_side"] = rhs->to_json();
    json.properties["operation_type"] = TokenTypeNames.at(op);
    return json;
}

cerne::JSON cerne::Parameter::to_json() {
    cerne::JSON json;
    json.properties["unpack"] = std::format("{}", unpack);
    json.properties["symbol_name"] = name;
    json.properties["parameter_type"] = type_to_json(ptype.get());
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
    json.properties["nodes"] = JSONBuilder{}.convert_array(nodes_json);
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
    json.properties["parameters"] = JSONBuilder{}.convert_array(params_json);
    json.properties["scope"] = body->to_json();
    json.properties["return_type"] = type_to_json(return_type.get());
    json.properties["name"] = name;
    json.properties["type"] = "FunNode";
    return json;
}

cerne::JSON cerne::VarDecl::to_json() {
    cerne::JSON json;
    json.properties["name"] = name;
    json.properties["is_const"] = std::format("{}", is_const);
    json.properties["uninitialized"] = std::format("{}", uninitialized);
    json.properties["var_type"] = type_to_json(var_type.get());
    json.properties["value"] = value ? JSONBuilder{value->to_json()}.build() : "null";
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
    json.properties["values"] = JSONBuilder{}.convert_array(values_json);
    json.properties["type"] = "ReturnStmt";
    return json;
}

cerne::JSON cerne::ImportNode::to_json() {
    cerne::JSON json;
    json.properties["file_path"] = file_path;
    json.properties["user"] = user;
    json.properties["package_path"] = JSONBuilder{}.convert_array(package_path);
    json.properties["is_path"] = std::format("{}", is_path);
    json.properties["is_package"] = std::format("{}", is_package);
    json.properties["is_from_user"] = std::format("{}", is_from_user);
    json.properties["type"] = "ImportNode";
    return json;
}

cerne::JSON cerne::ExportNode::to_json() {
    cerne::JSON json;
    json.properties["symbol"] = symbol;
    json.properties["type"] = "ExportNode";
    return json;
}

/* --- Node print methods End --- */