/*
    Cerne Compiler - Part of the Parser Component, handles the mnemonic & specific case scenario 

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#ifndef CE_PARSER_HANDLER
#define CE_PARSER_HANDLER

#include "../../../include/parser.hpp"
#include "../../../include/utils.hpp"

namespace cerne {
    /**
     * size_t <mnemonic name (with capital first letter)>(ast*, list, offset);
     * mnemonic blueprints return the amount of extra tokens read after offset
     * offset is the offset in which the mnemonic is localized in the token list
     */
    typedef union {
        std::unique_ptr<AST> ast;
        tokenlist list;
        size_t offset;
    } blueprint_arguments;

    size_t Fun(const blueprint_arguments& args);
    size_t Return(const blueprint_arguments& args);
}

#endif