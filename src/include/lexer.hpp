/*
    Cerne Compiler - Lexer Header

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#ifndef CE_LEXER
#define CE_LEXER

#include "utils.hpp"

#include<string_view>

namespace cerne {
    enum class TokenTypes {
        // words & numbers
        IDENTIFIER,
        MNEMONIC,
        REGISTER,
        NUMBER,

        // arithmetic
        PLUS,           // +
        MINUS,          // -
        DIV,            // /
        MUL,            // *

        // bit-related
        BIT_AND,        // &
        BIT_OR,         // |
        BIT_XOR,        // ^
        BIT_NOT,        // !

        // [], (), {}
        START_INDEX,    // [
        END_INDEX,      // ]
        START_PARAM,    // (
        END_PARAM,      // )
        START_SCOPE,    // {
        END_SCOPE,      // }

        // strings
        STRING,         // "" raw normal string
        FSTRING,        // '' formatted string
        SSTRING,        // `` used mainly with std.special (it holds a lot of metadata)

        // misc
        COMMA,          // ,
        DOT,            // .

        // conjectures
        ARROW,
        MEMBER_ACCESS,      // ::
        START_RULE,         // #rule
        START_MLC,          // /*
        START_COMMENT       // //
    };

    typedef struct Token {
        TokenTypes type;
        void* value;
        size_t line;
        size_t col;
    } Token;

    const std::vector<std::string> keywords = {
        "const", "let", 
        "fun", 
        "import", "export", "from", 
        "if", "elif", "else", 
        "while", "for", "in", "until", 
        "return", 
        "match", "break"
    };

    const std::vector<std::string> registers = {
        "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "rsp", "rbp",
        "eax", "ebx", "ecx", "edx", "esi", "edi", "esp", "ebp",
        "ax", "bx", "cx", "dx", "si", "di", "sp", "bp",
        "al", "bl", "cl", "dl", "sil", "dil", "spl", "bpl",
        "ah", "bh", "ch", "dh", "sih", "dih", "sph", "bph",
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
    };

    typedef std::vector<Token> tokenlist;

    tokenlist lexer(const std::string_view& code, const char* file_path, const args& options);
}

#endif