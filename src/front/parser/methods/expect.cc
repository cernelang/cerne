/*
    Cerne Compiler - component of the parser.

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#include "../../../include/parser.hpp"

/**
 * Utility which peeks, checks for eof and reports an error (if allowed to do so) 
 * returns whether the expected token was found or not
 */
bool cerne::ParseMachine::expect(TokenTypes type, bool just_check) {
    if(is_eof() && options.flags.find("quiet") == options.flags.end()) {
        cerne::cerror(
            file_path,
            ERR_UNEXPECTED_EOF,
            std::format("Expected {}, but reached end of file.", TokenTypeNames.at(type)),
            "EOF",
            Span{
                .line = 0,
                .col = 0,
                .offset = code_sv.size(),
                .length = 0
            }
        );
        errors++;
        return false;
    }

    const auto& token = peek();
    if(token.type != type) {
        if(!just_check && !options.flags.contains("quiet")) {
            cerne::cerror(
                file_path,
                ERR_UNEXPECTED_TOKEN,
                std::format("Expected {}, instead got {} at {}:{}", TokenTypeNames.at(type), TokenTypeNames.at(token.type), token.span.line, token.span.col),
                    cerne::code_snippet(
                        code_sv,
                        token.span,
                        std::format("`{}` is not a {}.", token.value ? *(token.value) : TokenTypeNames.at(token.type), TokenTypeNames.at(type))
                    ),
                token.span
            );

            skip_to_next_end();
            errors++;
        }

        return false;
    }
    return true;
}

/**
 * Same as expect but checks for multiple expected token types instead of just one
 */
bool cerne::ParseMachine::expect_or(std::vector<TokenTypes> types, bool just_check) {
    for(const auto& type : types) {
        if(expect(type, true)) {
            return true;
        }
    }

    if(!just_check && !options.flags.contains("quiet")) {
        const auto& token = peek();
        std::string expected_types_str;
        for(size_t i = 0; i < types.size(); i++) {
            expected_types_str += TokenTypeNames.at(types[i]);
            if(i < types.size() - 1) {
                expected_types_str += "` or `";
            }
        }
        
        cerne::cerror(
            file_path,
            ERR_UNEXPECTED_TOKEN,
            std::format("Expected one of `{}`, instead got {} at {}:{}", expected_types_str, TokenTypeNames.at(token.type), token.span.line, token.span.col),
            cerne::code_snippet(
                code_sv,
                token.span,
                std::format("`{}` is not one of `{}`.", token.value ? *(token.value) : TokenTypeNames.at(token.type), expected_types_str)
            ),
            token.span
        );

        skip_to_next_end();
        errors++;
    }
    
    return false;
}
