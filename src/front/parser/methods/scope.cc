/*
    Cerne Compiler - component of the parser.

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#include "../../../include/parser.hpp"

/**
 * Subparse method responsible for parsing a scope
 */
std::unique_ptr<cerne::Scope> cerne::ParseMachine::parse_scope() {
    auto& start_token = list[offset];
    offset++;

    auto scope = std::make_unique<cerne::Scope>();

    // parse everything until end of scope
    while(list[offset].type != TokenTypes::END_SCOPE && offset < list.size()) {
        auto node = parse(list[offset]);
        if(node) scope->body.push_back(std::move(node));
        offset++;
    }
    
    // if file ended without closing scope
    if(list[offset].type != TokenTypes::END_SCOPE && offset >= list.size() && options.flags.find("quiet") == options.flags.end()) {
        cerne::cerror(
            file_path,
            ERR_OPEN_SCOPE,
            std::format("Syntax Error: Unclosed scope. Expected {}'}}'{} to match the opening brace.", BG "219m" FG "233m", RESET ESC "[1;37m"),
            cerne::code_snippet(
                code_sv,
                start_token.span,
                "Scope begins here without closure (no matching '}' for this '{')"
            ),
            start_token.span
        );
        errors++;
    }

    return scope;
}