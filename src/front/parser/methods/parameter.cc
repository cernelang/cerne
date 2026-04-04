/*
    Cerne Compiler - component of the parser.

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#include "../../../include/parser.hpp"


// --- parse_parameter() Utils ---

bool checkIdentifier(cerne::ParseMachine* machine, const cerne::Token& id) {
    if(id.type != cerne::TokenTypes::IDENTIFIER  && machine->options.flags.find("quiet") == machine->options.flags.end()) {
        cerne::cerror(
            machine->file_path, 
            ERR_UNEXPECTED_TOKEN,
            std::format("Expected {} or {}, instead got {} at {}:{}", 
                cerne::TokenTypeNames.at(cerne::TokenTypes::UNPACK), 
                cerne::TokenTypeNames.at(cerne::TokenTypes::IDENTIFIER), 
                cerne::TokenTypeNames.at(id.type), 
                id.span.line, 
                id.span.col
            ), 
            cerne::code_snippet(
                machine->code_sv,
                id.span,
                std::format("`{}` is not a valid parameter name.", cerne::TokenTypeNames.at(id.type))
            ),
            id.span
        );
        machine->errors++;
        return false;
    }

    return true;
}

std::unique_ptr<cerne::Parameter> define_parameter(bool isunpack, const cerne::Token& token) {
    // initialize a parameter
    auto param = std::make_unique<cerne::Parameter>(
        token.span,
        isunpack,
        *token.value.get()
    );

    // change the parameter type to "any" by default
    param->ptype = std::make_unique<cerne::Type>(cerne::Type{
        .data = cerne::TypeData::PRIMITIVE,
        .typeinfo = cerne::Primitive::Any
    });

    return param;
}

/**
 * Helper util to update the parameter node with type information in case there is any
 */
void update_parameter(cerne::ParseMachine* machine, cerne::Parameter* param) {
    const auto& define = machine->list[machine->offset];

    // if there isn't a type in front of the parameter identifier (like param_name instead of param_name: int), just return
    if(define.type != cerne::TokenTypes::DEFINE) return;

    // if there is, we move on to the next token, check for EOF and then parse the type and update the parameter's ptype
    machine->offset++;

    if(!machine->expect(cerne::TokenTypes::IDENTIFIER)) return;

    auto type = machine->parse_type(false);
    if(type->data != cerne::TypeData::UNKNOWN) {
        param->ptype=std::move(type);
    }
}

/**
 * Parameter syntax:
 * <>ParamName: int         (all following parameters are of type integer)
 * MyParam: int             (MyParam is of type int)
 * Parameter                (when no type is explicitly declared, compiler assumes "any")
 */
std::unique_ptr<cerne::Parameter> cerne::ParseMachine::parse_parameter() {
    // first token MUST BE an unpack token or an identifier.
    if(!expect(TokenTypes::UNPACK) && !expect(TokenTypes::IDENTIFIER)) return nullptr;

    const auto& first = list[offset];

    if(first.type == TokenTypes::UNPACK) {
        offset++;
        if(!expect(TokenTypes::IDENTIFIER)) return nullptr;

        const auto& id = list[offset];
        // get identifier (if the next token is one) and define the parameter and push it
        if(!checkIdentifier(this, id)) return nullptr;
        auto param = define_parameter(true, id);
        update_parameter(this, param.get());
        return param;
    }

    if(!checkIdentifier(this, first)) return nullptr;

    auto param = define_parameter(false, first);
    update_parameter(this, param.get());
    return param;
}