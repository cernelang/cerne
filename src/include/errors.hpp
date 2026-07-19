/*
    Cerne Compiler - header including all error codes

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#ifndef CE_ERRORS
#define CE_ERRORS

#include<cstdint>

// lexer errors
constexpr uint8_t ERR_UNEXPECTED_SYMBOL      = 0x1;
constexpr uint8_t ERR_TOO_MANY_DOTS          = 0x2;
constexpr uint8_t ERR_UNEXPECTED_TOKEN       = 0x3;

// parser errors
constexpr uint16_t PARSER_SIGNATURE = 0x100;
constexpr uint16_t ERR_UNKNOWN_KEYWORD        = PARSER_SIGNATURE | 0x1;
constexpr uint16_t ERR_OPEN_SCOPE             = PARSER_SIGNATURE | 0x2;
constexpr uint16_t ERR_UNEXPECTED_EOF         = PARSER_SIGNATURE | 0x3;
constexpr uint16_t ERR_MALFORMED_IMPORT       = PARSER_SIGNATURE | 0x4;
constexpr uint16_t ERR_ELIF_OUTSIDE_IF        = PARSER_SIGNATURE | 0x5;
constexpr uint16_t ERR_ELSE_OUTSIDE_IF        = PARSER_SIGNATURE | 0x6;
constexpr uint16_t ERR_INVALID_LOOP_CONDITION = PARSER_SIGNATURE | 0x7;
constexpr uint16_t ERR_MISSING_LOOP_BODY      = PARSER_SIGNATURE | 0x8;
constexpr uint16_t ERR_MISSING_LOOP_VARIABLE  = PARSER_SIGNATURE | 0x9;
constexpr uint16_t ERR_MISSING_LOOP_CONDITION = PARSER_SIGNATURE | 0xa; // fundamentally different from ERR_INVALID_LOOP_CONDITION
constexpr uint16_t ERR_MISSING_LOOP_UPDATE    = PARSER_SIGNATURE | 0xb;
constexpr uint16_t ERR_LOOP_OPEN              = PARSER_SIGNATURE | 0xc;
constexpr uint16_t ERR_MALFORMED_LOOP         = PARSER_SIGNATURE | 0xd;

// SEMA errors
constexpr uint16_t SEMA_SIGNATURE = 0x200;
constexpr uint16_t ERR_REDECLARED_VARIABLE    = SEMA_SIGNATURE | 0x1;
constexpr uint16_t ERR_REDECLARED_FUNCTION    = SEMA_SIGNATURE | 0x2;

#endif