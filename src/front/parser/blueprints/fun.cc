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
std::unique_ptr<cerne::Node> cerne::Fun(const cerne::blueprint_arguments& args) {
    // store the token list and offset
    const auto& machine = args.machine;
    const auto& list = machine->list;
    machine->offset++;

    // check EOF
    if(machine->check_eof("Identifier")) return nullptr;

    // identifier
    const auto& id = list[machine->offset];
    if(id.type != cerne::TokenTypes::IDENTIFIER && machine->options.flags.find("quiet") == machine->options.flags.end()) {
        cerne::cerror(
            machine->file_path, 
            ERR_UNEXPECTED_TOKEN,
            std::format("Expected Identifier, instead got {} at {}:{}", cerne::TokenTypeNames.at(id.type), id.span.line, id.span.col), 
            cerne::code_snippet(
                machine->code_sv,
                id.span,
                std::format("`{}` is not a valid function name.", *(id.value))
            ),
            id.span
        );
        machine->offset++;
        machine->errors++;
        return nullptr;
    }

    // increment and get to the next expected token
    machine->offset++;

    // check EOF
    if(machine->check_eof("`(`")) return nullptr;

    // now we parse the parameters, which start with a START_PARAM token and end with an END_PARAM, separated by a COMMA token.
    const auto& start_param = list[machine->offset];
    if(start_param.type != cerne::TokenTypes::START_PARAM  && machine->options.flags.find("quiet") == machine->options.flags.end()) {
        cerne::cerror(
            machine->file_path, 
            ERR_UNEXPECTED_TOKEN,
            std::format("Expected a `(`, instead got `{}` at {}:{}", cerne::TokenTypeNames.at(start_param.type), start_param.span.line, start_param.span.col), 
            cerne::code_snippet(
                machine->code_sv,
                start_param.span,
                std::format("`{}` is not a valid start of function parameters.", cerne::TokenTypeNames.at(start_param.type))
            ),
            start_param.span
        );
        machine->errors++;
        return nullptr;
    }

    // increment again and start parsing parameters until a comma (or an END_PARAM) hits
    machine->offset++;

    std::vector<std::unique_ptr<Parameter>> parameters;
    while(list[machine->offset].type != cerne::TokenTypes::END_PARAM && machine->offset < machine->list.size()) {
        parameters.push_back(machine->parse_parameter());
    }

    machine->offset++;

    if(machine->check_eof("{")) return nullptr;

    // make a default type (void)
    auto fun_type = std::make_unique<Type>(
        TypeData::PRIMITIVE,
        false,
        false,
        Primitive::Void,
        nullptr
    );

    // check if explicit return type is provided (if there is an arrow)
    const auto& arrow = list[machine->offset];
    if(arrow.type == cerne::TokenTypes::ARROW) {
        machine->offset++;
        fun_type = machine->parse_type(false);
    }

    // now it should parse the scope
    const auto& begin_scope = list[machine->offset];
    auto scope = std::make_unique<Scope>();
    if(begin_scope.type == cerne::TokenTypes::START_SCOPE) {
        scope = std::move(machine->parse_scope());
    } else {
        cerne::cerror(
            machine->file_path, 
            ERR_UNEXPECTED_TOKEN,
            std::format("Expected `{{`, instead got `{}` at {}:{}", cerne::TokenTypeNames.at(begin_scope.type), begin_scope.span.line, begin_scope.span.col), 
            cerne::code_snippet(
                machine->code_sv,
                begin_scope.span,
                std::format("`{}` is not a valid start of function body.", cerne::TokenTypeNames.at(begin_scope.type))
            ),
            begin_scope.span
        );
        machine->errors++;
        return nullptr;
    }

    // create the function node and push it to the AST
    auto fun = std::make_unique<cerne::FunNode>(
        id.span,
        std::move(parameters),
        std::move(scope),
        std::move(fun_type),
        *id.value.get()
    );

    return fun;
}