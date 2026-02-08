/*
    Cerne Compiler - Parser Component, responsible for converting the token list from the previous lexer component to a more comprehensible node tree (AST).

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#ifndef CE_PARSER
#define CE_PARSER

#include "utils.hpp"
#include "lexer.hpp"

namespace cerne {
    enum class NodeTypes {
        PROGRAM,
        STATEMENT,
        EXPRESSION,
        LITERAL,
        FUNC_CALL,
        FUNC_DECL,
        VAR_DECL,
        IF, ELIF, ELSE,
        LOOP,
        SPACE_DECL,
        OBJECT_DECL
    };
    
    class Node {
        NodeTypes type;

    };

    
}

#endif