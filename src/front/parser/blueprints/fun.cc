/*
    Cerne Compiler - fun mnemonic blueprint for node construction

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#include "../../../include/parser/handler.hpp"

/**
 * fun blueprint:
 * fun <name>(<params>) -> <return type> <scope>
 * 
 * if no arrow and return type are provided, the return type is set to void by default
 */
std::unique_ptr<cerne::Node> cerne::Fun(const cerne::blueprint_arguments& args) {
    // store the token list and offset
    const auto& machine = args.machine;
    machine->advance();

    // check next token if it's an identifier
    if(!machine->expect(TokenTypes::IDENTIFIER)) return nullptr;

    // identifier
    const auto& id = machine->peek();

    // increment and get to the next expected token
    machine->advance();

    // now we parse the parameters, which start with a START_PARAM token and end with an END_PARAM, separated by a COMMA token.
    if(!machine->expect(TokenTypes::START_PARAM)) return nullptr;

    // increment again and start parsing parameters until a comma (or an END_PARAM) hits
    machine->advance();

    std::vector<std::unique_ptr<Parameter>> parameters;
    while(machine->offset < machine->list.size() && machine->peek().type != cerne::TokenTypes::END_PARAM) {
        parameters.push_back(machine->parse_parameter());
    }

    machine->advance();

    // functions can have an optional return type, which is specified using an ARROW token followed by the type. If no return type is specified, it defaults to void.
    if(!machine->expect_or({TokenTypes::ARROW, TokenTypes::START_SCOPE})) return nullptr;

    // make a default type (void)
    auto fun_type = std::make_unique<Type>(
        TypeData::PRIMITIVE,
        false,
        false,
        Primitive::Void,
        nullptr
    );

    // check if explicit return type is provided (if there is an arrow)
    const auto& arrow = machine->peek();
    if(arrow.type == cerne::TokenTypes::ARROW) {
        machine->advance();
        fun_type = machine->parse_type(false);
    }

    // now it should parse the scope
    const auto& begin_scope = machine->peek();
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