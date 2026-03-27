/*
    Cerne Compiler - mnemonic for returning (blueprint for the node)

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#include "../../../include/parser/handler.hpp"

/**
 * Return statements are just
 * return <expr>,<expr>... <end>
 * so we loop until there's an end token, parse the expressions in between and push them into the returnstmt node and push to the AST
 */
std::unique_ptr<cerne::Node> cerne::Return(const cerne::blueprint_arguments& args) {
    const auto& machine = args.machine;

    machine->offset++;
    if(machine->is_eof()) {
        cerne::cerror(
            machine->file_path,
            ERR_UNEXPECTED_EOF,
            "Expected an expression after `return`, but reached end of file.",
            "EOF",
            Span{
                .line = 0,
                .col = 0,
                .offset = machine->code_sv.size(),
                .length = 0
            }
        );
        machine->skip_to_next_end();
        machine->errors++;
        return nullptr;
    }

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
            cerne::cerror(
                machine->file_path,
                ERR_UNEXPECTED_TOKEN,
                std::format("Expected `COMMA`, `END` or `END_SCOPE`, but got {} at {}:{}", cerne::TokenTypeNames.at(current_token.type), current_token.span.line, current_token.span.col),
                cerne::code_snippet(machine->code_sv, current_token.span, std::format("Expected `COMMA`, `END` or `END_SCOPE`, but got {}.", cerne::TokenTypeNames.at(current_token.type))),
                current_token.span
            );
            machine->errors++;
            break;
        }
    }

    return return_node;
}