/*
    Cerne Compiler - part of the Parser component, responsible for parsing expressions using the Pratt Parsing method.

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#include "../include/parser.hpp"

/**
 * Used for pratt parsing
 * higher score means higher precedence
 * operators with the same score are left-associative, except for POWER which is right-associative
 */
[[nodiscard]] constexpr size_t cerne::get_score(cerne::TokenTypes type) noexcept {
    switch(type) {
        // conjectures & high precedence symbols
        case TokenTypes::DOT:
        case TokenTypes::MEMBER_ACCESS:
            return 11;
        case TokenTypes::POWER:
            return 10;

        // arithmetic
        case TokenTypes::MUL:
        case TokenTypes::DIV:
            return 9;
        case TokenTypes::PLUS:
        case TokenTypes::MINUS:
            return 8;

        // bitwise
        case TokenTypes::BIT_AND:
        case TokenTypes::BIT_OR:
        case TokenTypes::BIT_XOR:
        case TokenTypes::BIT_NOT:
            return 7;

        // no precedence tokens
        default:
            return 0;
    }
}

/**
 * Uses pratt parsing to parse expressions
 */
void cerne::ParseMachine::parse_expr(size_t precedence) {
    
}