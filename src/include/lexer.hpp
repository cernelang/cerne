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

namespace cerne {
    /**
     * All token types in cerne
     */
    enum class TokenTypes {
        // words & numbers
        IDENTIFIER,
        MNEMONIC,
        REGISTER,
        NUMBER,

        // arithmetic
        PLUS,
        MINUS,
        DIV,
        MUL,

        // bit-related
        BIT_AND,        // &
        BIT_OR,         // |
        BIT_XOR,        // ^
        BIT_NOT,        // !

        // compairison operators
        GREATER_THAN,   // <
        LESS_THAN,      // >

        // [], (), {}
        START_INDEX,
        END_INDEX,
        START_PARAM,
        END_PARAM,
        START_SCOPE,
        END_SCOPE,

        // strings
        STRING,         // "" raw normal string
        FSTRING,        // '' formatted string
        SSTRING,        // `` used mainly with std.special (it holds a lot of metadata)

        // misc
        COMMA,
        DOT,
        END,
        EQU,
        DEFINE,         // :

        // conjectures
        ARROW,
        POWER,
        UNPACK,             // <>
        RANGE,              // ..
        MEMBER_ACCESS,      // ::
        START_RULE,         // #!rule
        START_MLC,
        START_COMMENT,
        LEFT_SHIFT,         // <<
        RIGHT_SHIFT,        // >>
        GREATER_EQUAL,      // >=
        LESS_EQUAL,         // <=
        EQUAL,              // ==
        NOT_EQUAL,          // !=
        PIPELINE,           // |>
        OR,                 // ||
        AND,                // &&

        // reserved
        _EOF
    };

    /**
     * Convert token types to their string representation
     */
    const std::map<TokenTypes, std::string> TokenTypeNames = {
        { TokenTypes::IDENTIFIER, "Identifier" },
        { TokenTypes::MNEMONIC, "Mnemonic" },
        { TokenTypes::REGISTER, "Register" },
        { TokenTypes::NUMBER, "Number" },

        { TokenTypes::PLUS, "Plus" },
        { TokenTypes::MINUS, "Minus" },
        { TokenTypes::DIV, "Div" },
        { TokenTypes::MUL, "Mul" },
        
        { TokenTypes::BIT_AND, "BitAnd" },
        { TokenTypes::BIT_OR, "BitOr" },
        { TokenTypes::BIT_XOR, "BitXor" },
        { TokenTypes::BIT_NOT, "BitNot" },

        { TokenTypes::GREATER_THAN, "GreaterThan" },
        { TokenTypes::LESS_THAN, "LessThan" },
        
        { TokenTypes::START_INDEX, "StartIndex" },
        { TokenTypes::END_INDEX, "EndIndex" },
        { TokenTypes::START_PARAM, "StartParam" },
        { TokenTypes::END_PARAM, "EndParam" },
        { TokenTypes::START_SCOPE, "StartScope" },
        { TokenTypes::END_SCOPE, "EndScope" },

        { TokenTypes::STRING, "String" },
        { TokenTypes::FSTRING, "FormattedString" },
        { TokenTypes::SSTRING, "SpecialString" },

        { TokenTypes::COMMA, "Comma" },
        { TokenTypes::DOT, "Dot" },
        { TokenTypes::END, "End" },
        { TokenTypes::EQU, "Equal" },
        { TokenTypes::DEFINE, "Define" },

        { TokenTypes::ARROW, "Arrow" },
        { TokenTypes::POWER, "Power" },
        { TokenTypes::UNPACK, "Unpack" },
        { TokenTypes::RANGE, "Range" },
        { TokenTypes::MEMBER_ACCESS, "MemberAccess" },
        { TokenTypes::START_RULE, "StartRule" },
        { TokenTypes::START_MLC, "StartMLC" },
        { TokenTypes::START_COMMENT, "StartComment" },
        { TokenTypes::LEFT_SHIFT, "LeftShift" },
        { TokenTypes::RIGHT_SHIFT, "RightShift" },
        { TokenTypes::GREATER_EQUAL, "GreaterEqual" },
        { TokenTypes::LESS_EQUAL, "LessEqual" },
        { TokenTypes::EQUAL, "Equal" },
        { TokenTypes::NOT_EQUAL, "NotEqual" },
        { TokenTypes::PIPELINE, "Pipeline" },
        { TokenTypes::OR, "Or" },
        { TokenTypes::AND, "And" },

        { TokenTypes::_EOF, "EOF" }
    };

    /**
     * Allowed unary operators
     */
    const std::vector<TokenTypes> unary = {
        TokenTypes::MINUS,      // unary minus
        TokenTypes::BIT_NOT,    // negation
        TokenTypes::MUL,        // pointers
        TokenTypes::BIT_AND     // reference
    };

    /**
     * Right associative operators (for pratt parsing)
     */
    const std::vector<TokenTypes> right_associative = {
        TokenTypes::POWER
    };

    typedef struct Token {
        TokenTypes type;
        std::unique_ptr<std::string> value;
        Span span;

        // constructor and destructor
        Token(TokenTypes t, std::unique_ptr<std::string> v, Span s) : type(t), value(std::move(v)), span(s) {};
        ~Token()=default;

        // move constructor and move assignment operator
        Token(Token&&) noexcept = default;
        Token& operator=(Token&&) noexcept = default;

        // delete copy constructor and copy assignment operator to prevent accidental copying of tokens
        Token(const Token&) = delete;
        Token& operator=(const Token&) = delete;
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