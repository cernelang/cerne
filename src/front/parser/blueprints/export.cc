/*
    Cerne Compiler - Export node blueprint
    
    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#include "../../../include/parser/handler.hpp"

/**
 * Export should be used to export any symbol from the current module, whether it's a function, a variable, a class, etc...
 * syntax is very simple, with it being just
 * export <symbol>;
 */
std::unique_ptr<cerne::Node> cerne::Export(const blueprint_arguments& args) {
    const auto& machine = args.machine;

    machine->advance();

    // for now, the next token has to be an identifier
    if(!machine->expect(TokenTypes::IDENTIFIER)) {
        machine->errors++;
        return nullptr;
    }

    // current token is already the identifier so need to advance
    const auto& identifier = machine->peek();

    auto export_node = std::make_unique<ExportNode>(
        identifier.span,
        *(identifier.value)
    );

    return export_node;
}