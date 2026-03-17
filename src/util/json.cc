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
    std::string json = "{\n";
    json += std::format("    \"errors\": {},\n", ast->errors);
    json += std::format("    \"warnings\": {},\n", ast->warnings);
    json += std::format("    \"file_path\": \"{}\",\n", ast->file_path);
    json += "    \"root\": {\n";

    // now we will simply loop through the root node's node list and print each node (and their children)
    for(size_t i = 0; i < ast->root->node_list.size(); i++) {
        const auto& node = ast->root->node_list[i];
        json += std::format("        \"{}\": {{\n", cerne::NodeTypeNames.at(node->type));
        json += std::format("            \"span\": {{\n                \"line\": {},\n                \"col\": {},\n                \"offset\": {},\n                \"length\": {}\n            }},\n", node->span.line, node->span.col, node->span.offset, node->span.length);
        json += std::format("            \"metadata\": {}\n", node->print(16));
        json += "        }";
    }

    json += "\n    }\n}";
    return json;
}