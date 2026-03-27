/*
    Cerne Compiler - const mnemonic blueprint for node construction

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#include "../../../include/parser/handler.hpp"

std::unique_ptr<cerne::Node> cerne::_Const(const cerne::blueprint_arguments& args) {
    return commons::var_declaration(args, true);
}