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
#include "parser/ast.hpp"
#include "parser/types.hpp"

namespace cerne {
    /**
     * The parse machine class is responsible for 
     * containing the state of the parser
     * containing all the subparse methods
     * and also for walking through the token list and executing the appropriate subparse method depending on the current token
     * (it also contains the error/warning count for the AST)
     * (and it also contains the file path and the code string view for diagnostics)
     */
    class ParseMachine {
        public:
            size_t offset = 0;
            size_t errors = 0;
            size_t warnings = 0;
            size_t scope = 0;
            cerne::AST* ast;
            cerne::tokenlist& list;
            const char* file_path;
            const cerne::args& options;
            const std::string_view& code_sv;

            // constructor & destructor
            ParseMachine(
                cerne::AST* _ast, 
                cerne::tokenlist& _list, 
                const char* _file_path, 
                const cerne::args& _options,
                const std::string_view& _code_sv
            )
                : ast(_ast), list(_list), file_path(_file_path), options(_options), code_sv(_code_sv) {};
            ~ParseMachine()=default;

            // checks
            bool check_eof(const std::string_view& expected);

            // subparse methods
            std::unique_ptr<cerne::Node> parse_mnemonic();
            std::unique_ptr<cerne::Scope> parse_scope();
            std::unique_ptr<Parameter> parse_parameter();
            std::unique_ptr<Type> parse_type(bool is_nested);
            std::unique_ptr<cerne::Node> parse_nud();
            std::unique_ptr<cerne::Node> parse_infix(std::unique_ptr<cerne::Node> lhs);
            std::unique_ptr<cerne::Node> parse_expr(size_t precedence);
            std::unique_ptr<cerne::Node> parse(cerne::Token& token);
            void walk();
    };

    [[nodiscard]] constexpr size_t get_score(TokenTypes type) noexcept;
    std::unique_ptr<AST> parse(const std::string_view& code_sv, tokenlist& list, const char* file_path, const args& options);
}

#endif