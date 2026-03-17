/*
    Cerne Compiler - mnemonic for returning (blueprint for the node)

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#include "handler.hpp"

/**
 * Return statements are just
 * return <expr>,<expr>... <end>
 * so we loop until there's an end token, parse the expressions in between and push them into the returnstmt node and push to the AST
 */
void cerne::Return(const cerne::blueprint_arguments& args) {
    const auto& machine = args.machine;

    machine->offset++;
    if(machine->check_eof("`expr`")) return;

    const auto& token = machine->list[machine->offset];

    auto return_node = std::make_unique<cerne::ReturnStmt>(token.span);

    while(token.type != TokenTypes::END && token.type != TokenTypes::END_SCOPE) {
        auto expr = machine->parse_expr(0);
        
        const auto& current_token = machine->list[machine->offset];
        // if it's a comma, we push the expression
        if(current_token.type == TokenTypes::COMMA) {
            return_node->values.push_back(std::move(expr));
            machine->offset++;
        } else if(current_token.type == TokenTypes::END || current_token.type == TokenTypes::END_SCOPE) {
            return_node->values.push_back(std::move(expr));
            break;
        } else {
            cerne::expected(
                machine->file_path,
                current_token.span,
                "`COMMA` (,) | `END` (;|\\n) | `END_SCOPE` (})",
                cerne::TokenTypeNames.at(current_token.type)
            );
            machine->errors++;
            break;
        }
    }

    machine->ast->root->node_list.push_back(std::move(return_node));
}