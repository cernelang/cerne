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

cerne::JSON path_element_to_json(const cerne::BasicPathElement& element) {
    cerne::JSON json;
    
    // element's name and is_member
    json.properties["name"] = std::string(element.name);
    json.properties["is_member"] = std::format("{}", element.is_member);

    // go over all the modifiers (with for_each) and convert them to JSON
    std::vector<cerne::JSON> modifiers_json;
    std::ranges::for_each(element.modifiers, [&](const std::unique_ptr<cerne::Modifier>& modifier) {
        cerne::JSON modifier_json;
        modifier_json.properties["type"] = cerne::ModifierTypesNames.at(modifier->type);
        modifiers_json.push_back(modifier_json);
    });

    json.properties["modifiers"] = cerne::JSONBuilder{}.convert_array(modifiers_json);
    return json;
}

cerne::JSON path_to_json(const cerne::Path* path) {
    cerne::JSON json;
    std::vector<cerne::JSON> elements_json;

    // using for_each to iterate through each element in the path itself and then convert it to a JSON array
    std::ranges::for_each(path->basic_path, [&](const cerne::BasicPathElement& element) {
        elements_json.push_back(path_element_to_json(element));
    });

    // construct path itself's JSON representation
    json.properties["pure_path"] = std::format("{}", path->pure_path);
    json.properties["elements"] = cerne::JSONBuilder{}.convert_array(elements_json);
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
    json.properties["parameter_type"] = path_to_json(ptype.get());
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
    json.properties["nodes"] = cerne::JSONBuilder{}.convert_array(nodes_json);
    json.properties["type"] = "Scope";
    return json;
}

cerne::JSON cerne::FunNode::to_json() {
    cerne::JSON json;
    std::vector<cerne::JSON> params_json;

    std::ranges::for_each(parameters, [&](const std::unique_ptr<Parameter>& parameter) {
        params_json.push_back(parameter->to_json());
    });

    json.properties["parameters"] = JSONBuilder{}.convert_array(params_json);
    json.properties["scope"] = body->to_json();
    json.properties["return_type"] = path_to_json(return_type.get());
    json.properties["name"] = name;
    json.properties["type"] = "FunNode";
    return json;
}

cerne::JSON cerne::VarDecl::to_json() {
    cerne::JSON json;
    json.properties["name"] = name;
    json.properties["is_const"] = std::format("{}", is_const);
    json.properties["uninitialized"] = std::format("{}", uninitialized);
    json.properties["var_type"] = path_to_json(var_type.get());
    json.properties["value"] = value ? JSONBuilder{value->to_json()}.build() : "null";
    json.properties["type"] = "VarDecl";
    return json;
}

cerne::JSON cerne::ReturnStmt::to_json() {
    cerne::JSON json;
    std::vector<cerne::JSON> values_json;
    
    std::ranges::for_each(values, [&](const std::unique_ptr<Node>& value) {
        values_json.push_back(value->to_json());
    });

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