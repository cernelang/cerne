/*
    Cerne Compiler - else mnemonic blueprint for node construction

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#include "../../../include/parser/handler.hpp"

std::unique_ptr<cerne::Node> cerne::Else(const cerne::blueprint_arguments& args) {
    auto& machine = args.machine;
    cerne::cerror(
        machine->file_path, 
        ERR_ELSE_OUTSIDE_IF,
        std::format("`else` statement outside of a living `if` (aka conditional) block at {}:{}", machine->peek().span.line, machine->peek().span.col), 
        cerne::code_snippet(machine->code_sv, machine->peek().span, "`else` statement can only be defined inside of a living conditional block."),    
        machine->peek().span
    );
    machine->errors++;
    return nullptr;
}