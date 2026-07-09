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

    const auto& return_token = machine->peek();
    machine->advance();
    if(machine->is_eof()) {
        cerne::cerror(
            machine->file_path,
            ERR_UNEXPECTED_EOF,
            "Expected an expression after `return`, but reached end of file.",
            "EOF",
            return_token.span
        );
        machine->skip_to_next_end();
        machine->errors++;
        return nullptr;
    }

    auto return_node = std::make_unique<cerne::ReturnStmt>(return_token.span);

    while(machine->peek().type != TokenTypes::END && machine->peek().type != TokenTypes::END_SCOPE) {
        auto expr = machine->parse_expr(0);
        
        const auto& current_token = machine->peek();
        // if it's a comma, we push the expression
        if(current_token.type == TokenTypes::COMMA) {
            return_node->values.push_back(std::move(expr));
            machine->advance();
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

    // update span length
    return_node->span.length = (machine->peek(-1).span.offset + machine->peek(-1).span.length) - return_node->span.offset;
    return return_node;
}