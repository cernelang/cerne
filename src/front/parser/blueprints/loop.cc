/*
    Cerne Compiler - common loop blueprint (used since while and until have almost the same logic)

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#include "../../../include/parser/handler.hpp"

std::unique_ptr<cerne::Node> cerne::commons::loop(const cerne::blueprint_arguments& args, bool is_until) {
    auto& machine = args.machine;
    auto& token = machine->peek();

    auto previous_no_init = machine->no_init;
    machine->no_init = true; // disallow initialization in loops

    machine->advance(); // skip mnemonic

    machine->inside_stmt = true;
    auto condition = machine->parse_expr(0);
    machine->inside_stmt = false;

    if(!condition) {
        cerne::cerror(
            machine->file_path, 
            ERR_INVALID_LOOP_CONDITION,
            std::format("Invalid loop condition at {}:{}", machine->peek().span.line, machine->peek().span.col), 
            cerne::code_snippet(machine->code_sv, machine->peek().span, "Loop conditions must be valid expressions."),    
            machine->peek().span
        );
        machine->errors++;
        return nullptr;
    }

    auto body = machine->parse_scope();

    if(!body) {
        cerne::cerror(
            machine->file_path, 
            ERR_MISSING_LOOP_BODY,
            std::format("Missing loop body at {}:{}", machine->peek().span.line, machine->peek().span.col), 
            cerne::code_snippet(machine->code_sv, machine->peek().span, "Loop bodies must be valid scopes."),    
            machine->peek().span
        );
        machine->errors++;
        return nullptr;
    }

    auto& last_token = machine->peek(-1);

    // insert logical NOT into the start of expression by wrapping it in a prefix expression
    if(is_until) {
        condition = std::make_unique<cerne::PrefixExpr>(condition->span, std::move(condition), cerne::TokenTypes::NOT);
    }

    machine->no_init = previous_no_init; // restore previous no_init state
    
    return std::make_unique<cerne::WhileNode>(Span{
        .line=token.span.line,
        .col=token.span.col,
        .offset=token.span.offset,
        .length=(last_token.span.length + last_token.span.offset) - token.span.offset
    }, std::move(condition), std::move(body));
}