/*
    Cerne Compiler - sub-utility for Cerne's main diagnostics (specifically for the DebugEngine class)

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#include "../include/utils.hpp"

void cerne::DebugEngine::add_message(const std::string& message) {
    messages.push_back(message);
}

void pmsgs(const std::vector<std::string>& messages) {
    std::ranges::for_each(messages, [](const auto& message) {
        std::cerr << message << std::endl;
    });
}

/**
 * goes through each error and prints it
 */
void cerne::DebugEngine::print_messages(const std::vector<std::string>& messages) {
    pmsgs(messages);
}

