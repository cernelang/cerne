/*
    Cerne Compiler - Part of the Parser Component, handles the mnemonic & specific case scenario 

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#ifndef CE_PARSER_HANDLER
#define CE_PARSER_HANDLER

#include "../parser.hpp"
#include "../utils.hpp"

namespace cerne {
    /**
     * size_t <mnemonic name (with capital first letter)>(ast*, list, offset);
     * mnemonic blueprints return the amount of extra tokens read after offset
     * offset is the offset in which the mnemonic is localized in the token list
     */
    struct blueprint_arguments {
        cerne::ParseMachine* machine;
    };

    /**
     * Common blueprints are regular blueprints that simply hold the same logic that can be used for multiple mnemonics at the same time
     * One example is the variable declaration blueprint, the logic is the same for let and const, the only thing differing being mutability
     */
    namespace commons {
        std::unique_ptr<cerne::Node> var_declaration(const blueprint_arguments& args, bool is_const);
    }

    // function related blueprints
    std::unique_ptr<cerne::Node> Fun(const blueprint_arguments& args);
    std::unique_ptr<cerne::Node> Return(const blueprint_arguments& args);

    // variable declaration blueprints
    std::unique_ptr<cerne::Node> Let(const blueprint_arguments& args);
    // _Const because Const was giving me problems for some reason
    std::unique_ptr<cerne::Node> _Const(const blueprint_arguments& args);

    // package manager and module system blueprints
    std::unique_ptr<cerne::Node> Import(const blueprint_arguments& args);
    std::unique_ptr<cerne::Node> Export(const blueprint_arguments& args);
}

#endif