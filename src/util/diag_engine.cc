/*
    Cerne Compiler - sub-utility for Cerne's main diagnostics (specifically for the DEngine class)

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#include "../include/utils.hpp"

void cerne::DEngine::add_error(const CERROR& error) {
    errors.push_back(error);
}

void perrors(const std::vector<cerne::CERROR>& errors) {
    std::ranges::for_each(errors, [](const auto& error) {
        std::cerr << cerne::cerr_to_msg(error) << std::endl;
    });
}

/**
 * goes through each error and prints it
 */
void cerne::DEngine::print_errors(const std::vector<CERROR>& errors) {
    perrors(errors);
}

