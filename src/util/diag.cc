/*
    Cerne Compiler - Utility for Cerne's logging needs

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#include "../include/utils.hpp"
#include "../include/lexer.hpp"

void cerne::error(const char* src, const std::string& message, const std::string& code_snippet) {
    std::cout << FG "196;1m" << SIGNATURE << ' ' << src << ' ' << FG "255m" << message << '\n' << code_snippet << RESET << std::endl;
}

std::string cerne::code_snippet(const std::string_view& code, cerne::Span span) {
    std::string snippet = "";

    

    return snippet;
}

// log with a time
void cerne::tlog(double time, const std::string& message) {
    std::cout << FG "111;1m" SIGNATURE << FG "237m" " [" << FG "75m" << time << FG "237m" "] " << FG "255m" << message << std::endl;
}