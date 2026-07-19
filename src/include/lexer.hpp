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

        // native assembly
        ASM_STARTER,    // @

        // arithmetic
        PLUS,
        MINUS,
        DIV,
        MUL,
        MODULO,         // %

        // bit-related
        BIT_AND,        // &
        BIT_OR,         // |
        BIT_XOR,        // ^
        BIT_NOT,        // ~

        // logical operators (others are on the conjectures list)
        NOT,            // !

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
        ARROW,              // ->
        LAMBDA_ARROW,       // =>
        POWER,
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
        INCREMENT,          // ++
        DECREMENT,          // --
        PLUS_EQU,           // +=
        MINUS_EQU,          // -=
        DIV_EQU,            // /=
        MUL_EQU,            // *=
        BIT_AND_EQU,        // &=
        BIT_OR_EQU,         // |=
        BIT_XOR_EQU,        // ^=
        BIT_NOT_EQU,        // ~=
        NOT_EQUAL,          // !=
        PIPELINE,           // |>
        OR,                 // ||
        AND,                // &&

        // compounds (soon)
        LEFT_SHIFT_EQU,     // <<=
        RIGHT_SHIFT_EQU,    // >>=
        UNPACK,             // ...

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

        { TokenTypes::ASM_STARTER, "AsmStarter" },

        { TokenTypes::PLUS, "Plus" },
        { TokenTypes::MINUS, "Minus" },
        { TokenTypes::DIV, "Div" },
        { TokenTypes::MUL, "Mul" },
        { TokenTypes::MODULO, "Modulo" },
        
        { TokenTypes::BIT_AND, "BitAnd" },
        { TokenTypes::BIT_OR, "BitOr" },
        { TokenTypes::BIT_XOR, "BitXor" },
        { TokenTypes::BIT_NOT, "BitNot" },

        { TokenTypes::NOT, "Not" },

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
        { TokenTypes::EQU, "Equ" },
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
        { TokenTypes::INCREMENT, "Increment" },
        { TokenTypes::DECREMENT, "Decrement" },
        { TokenTypes::PLUS_EQU, "PlusEqual" },
        { TokenTypes::MINUS_EQU, "MinusEqual" },
        { TokenTypes::DIV_EQU, "DivEqual" },
        { TokenTypes::MUL_EQU, "MulEqual" },
        { TokenTypes::BIT_AND_EQU, "BitAndEqual" },
        { TokenTypes::BIT_OR_EQU, "BitOrEqual" },
        { TokenTypes::BIT_XOR_EQU, "BitXorEqual" },
        { TokenTypes::BIT_NOT_EQU, "BitNotEqual" },
        { TokenTypes::NOT_EQUAL, "NotEqual" },
        { TokenTypes::PIPELINE, "Pipeline" },
        { TokenTypes::OR, "Or" },
        { TokenTypes::AND, "And" },

        { TokenTypes::LEFT_SHIFT_EQU, "LeftShiftEqual" },
        { TokenTypes::RIGHT_SHIFT_EQU, "RightShiftEqual" },

        { TokenTypes::_EOF, "EOF" }
    };

    /**
     * Allowed unary operators
     */
    const std::vector<TokenTypes> unary = {
        TokenTypes::MINUS,      // unary minus
        TokenTypes::BIT_NOT,    // bit negation
        TokenTypes::BIT_XOR,    // bit exclusive or
        TokenTypes::BIT_OR,     // bit or
        TokenTypes::NOT,        // logical negation
        TokenTypes::MUL,        // de-reference
        TokenTypes::BIT_AND,    // reference
        TokenTypes::INCREMENT,  // pre-increment
        TokenTypes::DECREMENT   // pre-decrement
    };

    /**
     * Allowed suffix operators
     */
    const std::vector<TokenTypes> suffix = {
        TokenTypes::INCREMENT,  // post-increment
        TokenTypes::DECREMENT   // post-decrement
    };

    /**
     * Right associative operators (for pratt parsing)
     */
    const std::vector<TokenTypes> right_associative = {
        // exponentiation is right associative
        TokenTypes::POWER,

        // assignment operators
        TokenTypes::EQU,
        TokenTypes::PLUS_EQU,
        TokenTypes::MINUS_EQU,
        TokenTypes::DIV_EQU,
        TokenTypes::MUL_EQU,
        TokenTypes::BIT_AND_EQU,
        TokenTypes::BIT_OR_EQU,
        TokenTypes::BIT_XOR_EQU,
        TokenTypes::BIT_NOT_EQU,
        TokenTypes::LEFT_SHIFT_EQU,
        TokenTypes::RIGHT_SHIFT_EQU
    };

    struct Token {
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
    };

    const std::vector<std::string> keywords = {
        "const", "let", 
        "fun", 
        "import", "export", "from", 
        "if", "elif", "else", 
        "while", "for", "in", "until", 
        "return", 
        "match", "case"
    };

    const std::vector<std::string> registers = {
        "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "rsp", "rbp",
        "eax", "ebx", "ecx", "edx", "esi", "edi", "esp", "ebp",
        "ax", "bx", "cx", "dx", "si", "di", "sp", "bp",
        "al", "bl", "cl", "dl", "sil", "dil", "spl", "bpl",
        "ah", "bh", "ch", "dh", "sih", "dih", "sph", "bph",
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
    };

    using tokenlist = std::vector<Token>;

    
    class LexerMachine {
        public:
            std::vector<cerne::Token> tokens;
            cerne::TokenTypes string_subtype;
            size_t line = 1;
            size_t col = 0;
            size_t offset = 0;
            bool is_comment = false;
            bool is_mlc = false;
            bool is_string = false;
            bool number_began_in_dot = false;
            std::string raw_str_content = "";
            const std::string_view& code;
            const char* file_path;
            const cerne::args& options;

            LexerMachine(const std::string_view& code, const char* file_path, const cerne::args& options) : code(code), file_path(file_path), options(options) {};
            ~LexerMachine()=default;

            // utilities with the tokenlist and position updates
            void update(char c);
            void push(cerne::Token&& token);

            // actual tokens themselves
            bool comment(char c, char n);
            bool string(char c);
            void word(char c);
            void number(char c);
            void compound(const std::string& possible_compound);
            void conjecture(const std::string& possible_conjecture);
            void symbol(char c, char n);
    };

    tokenlist lexer(const std::string_view& code, const char* file_path, const args& options);
}

#endif