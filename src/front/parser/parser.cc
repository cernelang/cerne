/*
    Cerne Compiler - Parser Component, responsible for converting the token list from the previous lexer component to a more comprehensible node tree (AST).

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#include "../include/parser.hpp"
#include "blueprints/handler.hpp"

std::map<std::string, std::function<void(const cerne::blueprint_arguments&)>> blueprints = {
    { "return", static_cast<void(*)(const cerne::blueprint_arguments&)>(cerne::Return) },
    { "fun", static_cast<void(*)(const cerne::blueprint_arguments&)>(cerne::Fun) }
};

/* --- Parse Machine Methods Begin --- */

bool cerne::ParseMachine::check_eof(const std::string_view& expected) {
    if(offset >= list.size()) {
        const auto& last_element = list[list.size()-1];

        cerne::expected(
            file_path,
            last_element.span,
            expected,
            "EOF"
        );

        errors++;
        return true;
    }

    return false;
}

/**
 * Execute mnemonic blueprint
 */
void cerne::ParseMachine::parse_mnemonic() {
    const auto& token = list[offset];
    std::string token_name = std::string(token.value.get()->c_str());
    if(blueprints.find(token_name) != blueprints.end()) {
        auto arguments = cerne::blueprint_arguments{
            .machine=this
        };

        blueprints[token_name](arguments);
    } else {
        cerne::cerror(
            file_path, 
            3,
            std::format("Unknown keyword `{}` at {}:{}", token_name, token.span.line, token.span.col), 
            cerne::code_snippet(code_sv, token.span, std::format("`{}` does not exist yet.", *(token.value))),    
            token.span
        );
        errors++;
    }
}

void cerne::ParseMachine::parse_parameter() {

}

/**
 * Global parse function, executes appropriate function depending on the current token
 */
void cerne::ParseMachine::parse(const cerne::Token& token) {
    switch(token.type) {
        case cerne::TokenTypes::MNEMONIC: 
            parse_mnemonic();
            break;

        default:
            // how?? 
            break;
    }

    return;
}

void cerne::ParseMachine::walk() {
    for(; offset < list.size(); offset++) {
        const auto& token = list[offset];
        parse(token);
    }
}

/* --- Parse Machine Methods Over --- */

/* --- main parse function --- */

std::unique_ptr<cerne::AST> cerne::parse(const std::string_view& code_sv, const cerne::tokenlist& list, const char* path, const cerne::args& options) {
    // initialize the AST and the machine
    auto ast = std::make_unique<AST>(path);
    auto machine = std::make_unique<ParseMachine>(ast.get(), list, path, options, code_sv);

    // begin machine walk
    machine->walk();

    // before returning, pass onto the AST the errors/warnings (important for the CLI's main build loop)
    ast->errors     =   machine->errors;
    ast->warnings   =   machine->warnings;

    return ast;
}