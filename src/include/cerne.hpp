/*
    Cerne Compiler - Main Header File

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#ifndef CERNE
#define CERNE

// cerne libraries (by component)
#include "./utils.hpp"
#include "./lexer.hpp"
#include "./parser.hpp"
#include "./sema.hpp"

// cerne version update comes here for now
struct Version {
    int alpha;
    int major;
    int minor;
};

constexpr Version CERNE_VERSION{0, 2, 0};

#endif