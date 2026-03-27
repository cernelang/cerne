/*
    Cerne Compiler - variable declaration (common blueprint)

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#include "../../../include/parser/handler.hpp"

/**
 * Variable declarations follow a very simple process:
 * <let|const|type> <identifier> [: <type>] <end| `=` <expr> <end>>
 * if no types are specified (neither at the start nor after the identifier), the type is set to auto by default
 */
std::unique_ptr<cerne::Node> cerne::commons::var_declaration(const blueprint_arguments& args, bool is_const) {
    const auto& machine = args.machine;
    const auto& list = machine->list;

    // advance and check next token
    machine->advance();

    // check if it really is an identifier
    const auto& id = machine->peek();
    const auto var_name = *(id.value);

    // must be a token
    if(!machine->expect(TokenTypes::IDENTIFIER)) {
        machine->skip_to_next_end();
        return nullptr;
    }

    machine->advance();
    
    // (if there are no equals or type declarations right away, it's an unitialized "auto" variable)
    if(machine->offset >= list.size() || list[machine->offset].type == TokenTypes::END) {
        return std::make_unique<VarDecl>(
            id.span,
            var_name,
            is_const,
            true, // is uninitialized
            std::make_unique<Type>(Type{
                .data=TypeData::PRIMITIVE,
                .is_const=is_const,
                .is_pointer=false,
                .typeinfo=Primitive::Auto
            }),
            nullptr // no value since no initialization occured
        );
    }

    // now check for any explicit type declaration
    const auto& def = list[machine->offset];
    std::unique_ptr<Type> vartype = nullptr;

    if(def.type != TokenTypes::DEFINE) {
        vartype=std::make_unique<Type>(Type{
            .data=TypeData::PRIMITIVE,
            .is_const=is_const,
            .is_pointer=false,
            .typeinfo=Primitive::Auto
        });
    } else {
        machine->offset++;
        
        // after parse_type, offset will already be at the next token after type declaration, so no need to increment again
        vartype=machine->parse_type(false);
    }

    // check for an equal sign
    const auto& eq = list[machine->offset];

    // this is because we can have an unitialized TYPED variable, such as `let x: int`
    // but syntax rules apply for things like `let x: int _ ...` where _ is just an unexpected token in general

    if(!machine->expect_or({TokenTypes::EQU, TokenTypes::END})) return nullptr;

    std::unique_ptr<Node> value = nullptr;
    bool uninitialized = true;
    if(eq.type == TokenTypes::EQUAL) {
        machine->advance();
        value = machine->parse_expr(0);
        uninitialized = false;
    }

    // now build the variable declaration node
    auto vardec = std::make_unique<VarDecl>(
        id.span,
        var_name,
        is_const,
        uninitialized, 
        std::move(vartype),
        value ? std::move(value) : nullptr
    );

    return vardec;
}