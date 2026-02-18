/*
    Cerne Compiler - fun mnemonic blueprint for node construction

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#include "handler.hpp"

/**
 * fun blueprint:
 * fun <name>(<params>) -> <return type> <scope>
 * 
 * if no arrow and return type are provided, the return type is set to void by default
 */
void cerne::Fun(const cerne::blueprint_arguments& args) {
    // store the token list and offset
    const auto& machine = args.machine;
    const auto& list = machine->list;
    machine->offset++;

    // check EOF
    if(machine->check_eof("Identifier")) return;

    //  identifier
    const auto& id = list[machine->offset];
    if(id.type != cerne::TokenTypes::IDENTIFIER) {
        cerne::cerror(
            machine->file_path, 
            2,
            std::format("Expected Identifier, instead got {} at {}:{}", cerne::TokenTypeNames.at(id.type), id.span.line, id.span.col), 
            "",
            id.span
        );
        machine->offset++;
        machine->errors++;
        return;
    }

    // increment and get to the next expected token
    machine->offset++;

    // check EOF
    if(machine->check_eof("`(`")) return;

    // now we parse the parameters, which start with a START_PARAM token and end with an END_PARAM, separated by a COMMA token.
    const auto& start_param = list[machine->offset];
    if(start_param.type != cerne::TokenTypes::START_PARAM) {
        cerne::cerror(
            machine->file_path, 
            2,
            std::format("Expected a `(`, instead got `{}` at {}:{}", cerne::TokenTypeNames.at(start_param.type), start_param.span.line, start_param.span.col), 
            "",
            id.span
        );
        machine->errors++;
        return;
    }

    // increment again and start parsing parameters until a comma (or an END_PARAM) hits
    machine->offset++;

    const auto& current_token = list[machine->offset];
    while(current_token.type != cerne::TokenTypes::END_PARAM) {
        machine->parse_parameter();
        machine->offset++;
    }
}