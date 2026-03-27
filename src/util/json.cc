/*
    Cerne Compiler - Utility to convert AST Nodes to JSON for debugging and visualization

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#include "../include/utils.hpp"
#include "../include/parser.hpp"

std::string cerne::json(void* AST) {
    const auto& ast = static_cast<cerne::AST*>(AST);

    const auto& builder = std::make_unique<cerne::JSONBuilder>();
    std::vector<cerne::JSON> root_nodes;

    for(size_t i = 0; i < ast->root->node_list.size(); i++) {
        const auto& node = ast->root->node_list[i];
        root_nodes.push_back(node->to_json());
    }

    builder->add_property("errors", std::format("{}", ast->errors));
    builder->add_property("warnings", std::format("{}", ast->warnings));
    builder->add_property("file_path", std::string(ast->file_path));
    builder->add_property("root", builder->convert_array(root_nodes));

    const auto json_str = builder->build();
    return json_str;
}

std::string cerne::JSONBuilder::build() {
    std::string json_str = std::string(indentation_level * JSON_INDENT_SIZE, ' ') + "{\n";
    indentation_level++;

    for(auto it = json.properties.begin(); it != json.properties.end(); ++it) {
        const auto& [key, value] = *it;
        auto next_it = std::next(it);

        std::visit([&json_str, &key, this, &next_it](const auto& val) {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_same_v<T, std::string>) {

                json_str += std::format(
                    "{}\"{}\": {}{}{}{}\n", 
                    std::string(indentation_level * JSON_INDENT_SIZE, ' '), 
                    key, 
                    // arrays are exempt from quotes, since they are already valid JSON, string aren't though
                    ((val[0] == '[') ? ' ' : '"'), val, ((val[0] == '[') ? ' ' : '"'),
                    (next_it == json.properties.end() ? "" : ",")
                );
            
            } else if constexpr (std::is_same_v<T, cerne::JSON>) {

                json_str += std::format(
                    "{}\"{}\": {}{}\n", 
                    std::string(indentation_level * JSON_INDENT_SIZE, ' '), 
                    key, 
                    JSONBuilder{val, indentation_level+1}.build(), 
                    (next_it == json.properties.end() ? "" : ",")
                );

            }
        }, value);
    }

    indentation_level--;
    json_str += std::string(indentation_level * JSON_INDENT_SIZE, ' ') + "}";
    return json_str;
}

std::string cerne::JSONBuilder::convert_array(const std::variant<std::vector<std::string>, std::vector<JSON>>& arr) {
    std::string json_str = std::string(indentation_level * JSON_INDENT_SIZE, ' ') + "[\n";
    indentation_level++;

    std::visit([&json_str, this](const auto& val) {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, std::vector<std::string>>) {
            for(const auto& str : val) {
                json_str += std::format(
                    "{}\"{}\"{}\n", 
                    std::string(indentation_level * JSON_INDENT_SIZE, ' '), 
                    str,
                    (str != val.back() ? "," : "")
                );
            }
        } else if constexpr (std::is_same_v<T, std::vector<cerne::JSON>>) {
            for(size_t i = 0; i < val.size(); i++) {
                const auto& json = val[i];

                json_str += std::format(
                    "{}{}{}\n", 
                    std::string(indentation_level * JSON_INDENT_SIZE, ' '), 
                    JSONBuilder{json, indentation_level+1}.build(),
                    (i != (val.size() - 1) ? "," : "")
                );
            }
        }
    }, arr);

    indentation_level--;
    json_str += std::string(indentation_level * JSON_INDENT_SIZE, ' ') + "]";
    return json_str;
}

void cerne::JSONBuilder::add_property(const std::string& key, const std::variant<std::string, cerne::JSON>& value) {
    json.properties[key] = value;
}