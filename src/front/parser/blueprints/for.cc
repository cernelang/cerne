/*
    Cerne Compiler - blueprint for the "for loop" node construction

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#include "../../../include/parser/handler.hpp"

/**
 * requires token to be AT the first token of condition clause 
 * another important note - this is ONLY for C-style loops, not range loops
 * will return true if there is an error
 */
bool after_init(cerne::ParseMachine* machine, std::unique_ptr<cerne::ForNode>& for_node, const cerne::Token& token, bool has_parens) {
    machine->inside_stmt = true;

    auto condition_expr = machine->parse_expr(0);
    if(!condition_expr) {
        if(!machine->options.flags.contains("quiet")) {
            cerne::cerror(
                machine->file_path,
                ERR_MISSING_LOOP_CONDITION,
                std::format("Expected a condition expression for the for loop, instead got {}", cerne::TokenTypeNames.at(machine->peek().type)),
                cerne::code_snippet(machine->code_sv, machine->peek().span, "For loops must have a condition expression."),
                machine->peek(-1).span
            );
        }
        machine->errors++;
        machine->inside_stmt = false;
        return true;
    }
    for_node->condition = std::move(condition_expr);

    auto& end_token = machine->peek();
    if(end_token.type != cerne::TokenTypes::END) {
        if(!machine->options.flags.contains("quiet")) {
            cerne::cerror(
                machine->file_path,
                ERR_MALFORMED_LOOP,
                std::format("Malformed for loop, instead got {}", cerne::TokenTypeNames.at(machine->peek().type)),
                cerne::code_snippet(machine->code_sv, machine->peek().span, "For loops must have `;` or a new line separating the condition and update clauses."),
                machine->peek(-1).span
            );
        }
        machine->errors++;
        machine->inside_stmt = false;
        return true;
    }
    
    machine->advance(); // advance past the END token

    machine->inside_stmt = false;
    auto update_expr = machine->parse_expr(0);
    if(!update_expr) {
        if(!machine->options.flags.contains("quiet")) {
            cerne::cerror(
                machine->file_path,
                ERR_MISSING_LOOP_UPDATE,
                std::format("Expected an update expression for the for loop, instead got {}", cerne::TokenTypeNames.at(machine->peek().type)),
                cerne::code_snippet(machine->code_sv, machine->peek().span, "For loops must have an update expression."),
                machine->peek(-1).span
            );
        }
        machine->errors++;
        return true;
    }
    for_node->update = std::move(update_expr);

    // parameted c-style loops
    auto& end_param = machine->peek();
    if(has_parens) {
        if(end_param.type != cerne::TokenTypes::END_PARAM) {
            if(!machine->options.flags.contains("quiet")) {
                cerne::cerror(
                    machine->file_path,
                    ERR_LOOP_OPEN,
                    std::format("Expected a closing parenthesis for the for loop, instead got {}", cerne::TokenTypeNames.at(end_param.type)),
                    cerne::code_snippet(machine->code_sv, end_param.span, "Expected a )"),
                    end_param.span
                );
            }
            machine->errors++;
            return true;
        }

        machine->advance(); // advance past the END_PARAM token
    }

    // get body and return
    auto body_scope = machine->parse_scope();
    if(!body_scope) {
        if(!machine->options.flags.contains("quiet")) {
            cerne::cerror(
                machine->file_path,
                ERR_MISSING_LOOP_BODY,
                std::format("Expected a body for the for loop, instead got {}", cerne::TokenTypeNames.at(machine->peek().type)),
                cerne::code_snippet(machine->code_sv, machine->peek().span, "For loops must have a body, defined using {}"),
                machine->peek().span
            );
        }
        machine->errors++;
        return true;
    }

    for_node->body = std::move(body_scope);
    for_node->span = cerne::Span{
        token.span.line,
        token.span.col,
        token.span.offset,
        (for_node->body->span.offset + for_node->body->span.length) - token.span.offset
    };

    return false;
}

/**
 * for is actually quite complex structurally speaking, since it supports so many variations, here are some examples:
 * for x in list {}
 * for let i = 0; i < 10; i++ {}
 * for u8 i = 0; i < 10; i++ {}
 * for ; i < 10; i++ {} -> init clause is optional, so you can leave it empty
 * for auto x in list {}
 * not to forget all of these also have their () counterparts, to support
 * for(x in list) {}
 * syntax
 */
std::unique_ptr<cerne::Node> cerne::For(const cerne::blueprint_arguments& args) {
    auto machine = args.machine;

    machine->advance(); // advance past for

    auto& token = machine->peek();

    bool has_parens = false;
    if(token.type == cerne::TokenTypes::START_PARAM) {
        has_parens = true;
        machine->advance(); // advance past (
    }

    auto& clause_token = machine->peek();
    auto for_node = std::make_unique<cerne::ForNode>(clause_token.span);
    
    // END token means there's no init clause, its automatically assumed to be a C-style loop with no init clause
    if(clause_token.type == TokenTypes::END) {
        machine->advance(); // advance past END
        for_node->init = nullptr;

        if(after_init(machine, for_node, token, has_parens)) {
            return nullptr; // return if there was an error
        }

        return for_node;
    } 

    // first clause has to be ONLY either an expression or vardecl node
    auto first = machine->parse(machine->peek());
    if(!first || !(first->type == NodeType::LiteralExpr || first->type == NodeType::AssignmentExpr || first->type == NodeType::VarDecl)) {
        if(!machine->options.flags.contains("quiet")) {
            cerne::cerror(
                machine->file_path,
                ERR_INVALID_LOOP_CONDITION,
                std::format("Unexpected expression to begin a loop."),
                cerne::code_snippet(machine->code_sv, machine->peek().span, "Expected expressions, variable names or declarations such as 'let a = 1', 'x in ...' and others"),
                first ? first->span : machine->peek().span
            );
        }
        machine->errors++;
        return nullptr;
    }

    // if it passes, we now need to check if it's a range loop or an initialized c-style loop
    auto& next_token = machine->peek();

    if(next_token.type == cerne::TokenTypes::MNEMONIC) { // range loop
        if(*(next_token.value) == "in") {
            machine->advance(); // advance past in

            for_node->is_range_loop = true;

            // next should be a valid expression for the range
            machine->inside_stmt = true;
            auto range_expr = machine->parse_expr(0);
            machine->inside_stmt = false;

            if(!range_expr) {
                if(!machine->options.flags.contains("quiet")) {
                    cerne::cerror(
                        machine->file_path,
                        ERR_INVALID_LOOP_CONDITION,
                        std::format("Expected a range expression for the for loop, instead got {}", cerne::TokenTypeNames.at(machine->peek().type)),
                        cerne::code_snippet(machine->code_sv, machine->peek().span, "For loops must have a range expression."),
                        machine->peek(-1).span
                    );
                }
                machine->errors++;
                return nullptr;
            }

            // parameted range loops
            auto& end_param = machine->peek();
            if(has_parens) {
                if(end_param.type != cerne::TokenTypes::END_PARAM) {
                    if(!machine->options.flags.contains("quiet")) {
                        cerne::cerror(
                            machine->file_path,
                            ERR_LOOP_OPEN,
                            std::format("Expected a closing parenthesis for the for loop, instead got {}", cerne::TokenTypeNames.at(end_param.type)),
                            cerne::code_snippet(machine->code_sv, end_param.span, "Expected a )"),
                            end_param.span
                        );
                    }
                    machine->errors++;
                    return nullptr;
                }

                machine->advance(); // advance past the END_PARAM token
            }

            auto body_scope = machine->parse_scope();
            if(!body_scope) {
                if(!machine->options.flags.contains("quiet")) {
                    cerne::cerror(
                        machine->file_path,
                        ERR_MISSING_LOOP_BODY,
                        std::format("Expected a body for the for loop, instead got {}", cerne::TokenTypeNames.at(machine->peek().type)),
                        cerne::code_snippet(machine->code_sv, machine->peek().span, "For loops must have a body, defined using {}"),
                        machine->peek().span
                    );
                }
                machine->errors++;
                return nullptr;
            }

            // return everything
            for_node->variable = std::move(first);
            for_node->range = std::move(range_expr);
            for_node->body = std::move(body_scope);
            return for_node;
        }
    } else if(next_token.type == cerne::TokenTypes::END) { // C-Style with initialized clause
        machine->advance();
        for_node->init = std::move(first);

        if(after_init(machine, for_node, next_token, has_parens)) {
            return nullptr; // return if there was an error
        }

        return for_node;
    } else {
        if(!machine->options.flags.contains("quiet")) {
            cerne::cerror(
                machine->file_path,
                ERR_INVALID_LOOP_CONDITION,
                std::format("Unexpected token `{}` at {}:{}", *(next_token.value), next_token.span.line, next_token.span.col),
                cerne::code_snippet(machine->code_sv, next_token.span, std::format("`{}` is not a valid way to start a for loop. Expected `in` or `;`", *(next_token.value))),
                next_token.span
            );
        }
        machine->errors++;
        return nullptr;
    }

    return nullptr; // for now
}