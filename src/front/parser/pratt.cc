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
        case TokenTypes::INCREMENT:
        case TokenTypes::DECREMENT:
            return 17;
        case TokenTypes::POWER:
            return 15;

        // arithmetic
        case TokenTypes::MUL:
        case TokenTypes::DIV:
        case TokenTypes::MODULO:
            return 14;
        case TokenTypes::PLUS:
        case TokenTypes::MINUS:
            return 13;

        // bit shifts
        case TokenTypes::LEFT_SHIFT:
        case TokenTypes::RIGHT_SHIFT:
            return 12;

        // comparisons
        case TokenTypes::GREATER_THAN:
        case TokenTypes::LESS_THAN:
        case TokenTypes::GREATER_EQUAL:
        case TokenTypes::LESS_EQUAL:
            return 11;

        // equality
        case TokenTypes::EQUAL:
        case TokenTypes::NOT_EQUAL:
            return 10;
            
        // bitwise operators
        case TokenTypes::BIT_AND:
            return 9;
        case TokenTypes::BIT_XOR:
            return 8;
        case TokenTypes::BIT_OR:
            return 7;
            
        // logical
        case TokenTypes::AND:
            return 6;
        case TokenTypes::OR:
            return 5;

        // pipelines and ranges
        case TokenTypes::RANGE:
            return 4;
        case TokenTypes::PIPELINE:
            return 3;

        // all other assignment operators should have lowest precedence (below OR expressions)
        case TokenTypes::EQU:
        case TokenTypes::PLUS_EQU:
        case TokenTypes::MINUS_EQU:
        case TokenTypes::DIV_EQU:
        case TokenTypes::MUL_EQU:
        case TokenTypes::BIT_AND_EQU:
        case TokenTypes::BIT_OR_EQU:
        case TokenTypes::BIT_XOR_EQU:
        case TokenTypes::BIT_NOT_EQU:
        case TokenTypes::LEFT_SHIFT_EQU:
        case TokenTypes::RIGHT_SHIFT_EQU:
            return 2;

        // no precedence tokens
        default:
            return 0;
    }
}

/**
 * Also used for pratt parsing
 * more specialized than get_score, this function is used for unary operators, since they have a different precedence than their binary counterparts
 */
[[nodiscard]] constexpr size_t cerne::get_unary_score(cerne::TokenTypes type) noexcept {
    switch(type) {
        // unary increment and decrement aka pre-increment and pre-decrement have the highest precedence
        case TokenTypes::INCREMENT:
        case TokenTypes::DECREMENT:
        case TokenTypes::BIT_NOT:
        case TokenTypes::NOT:
        case TokenTypes::MUL:
        case TokenTypes::BIT_AND:
        case TokenTypes::MINUS:
        case TokenTypes::PLUS:
            return 16;

        default:
            return 0;
    }
}

std::unique_ptr<cerne::Node> cerne::ParseMachine::parse_nud(bool quiet) {
    // initialize token with the token at the current offset on the list
    auto& token = list[offset];
    auto type = token.type;

    switch (type) {
        // identifiers should call a LiteralExpr
        case TokenTypes::IDENTIFIER: {
            auto path = parse_path(false, quiet); // it already stops at the token after the path, so we don't need to advance here
            return std::make_unique<cerne::LiteralExpr>(token.span, std::move(path));
        }

        // for numbers and strings, just create a leaf node
        case TokenTypes::NUMBER:
            offset++;
            return std::make_unique<cerne::Leaf>(token.span, std::move(token.value), true);

        case TokenTypes::STRING:
        case TokenTypes::FSTRING:
        case TokenTypes::SSTRING:
            offset++;
            return std::make_unique<cerne::Leaf>(token.span, std::move(token.value), false);
        
        case TokenTypes::START_PARAM: {
            // advanced and call expr again
            offset++;

            auto prev_no_init = no_init;
            no_init = false; // support initialization since it's inside of a guarded context

            auto params = parse_expr(0, nullptr, quiet);

            no_init = prev_no_init; // restore no_init to its previous state

            // after expression is parsed, check if the token it stopped at is a )
            // groups should begin with ( and end with )
            if(!expect(TokenTypes::END_PARAM)) break;

            offset++;

            // (path).something
            auto& token = peek();
            if(token.type == TokenTypes::DOT || token.type == TokenTypes::MEMBER_ACCESS) {
                advance();
                auto path = parse_path(false, quiet);

                // anything other than ().identifier<path> is invalid syntax
                if(!path) {
                    if(!quiet || !options.flags.contains("quiet")) {
                        cerne::cerror(
                            file_path,
                            ERR_UNEXPECTED_SYMBOL,
                            std::format("Unexpected token `{}` at {}:{}", *(token.value), token.span.line, token.span.col),
                            cerne::code_snippet(code_sv, token.span, std::format("`{}` is not a valid way to access a member. (expected identifier, got {})", *(token.value), TokenTypeNames.at(token.type))),
                            token.span
                        );
                    }
                    errors++;
                    return nullptr;
                }

                path->base = std::move(params);
                path->basic_path[0].is_member = (token.type == TokenTypes::MEMBER_ACCESS);
                return std::make_unique<cerne::LiteralExpr>(token.span, std::move(path));
            }

            return params;
        }

        default:
            // unary
            if(std::ranges::find(unary, type) != unary.end()) {
                // consume unary
                offset++;
                
                // get right hand side of unary expression
                auto right = parse_expr(get_unary_score(type), nullptr, quiet);

                // create a prefix expression node
                return std::make_unique<cerne::PrefixExpr>(token.span, std::move(right), type); // type is op in this case
            } 

            // invalid expression starter, generate error and return nullptr
            if(!quiet || !options.flags.contains("quiet")) {
                cerne::cerror(
                    file_path,
                    ERR_UNEXPECTED_SYMBOL,
                    std::format("Unexpected token `{}` at {}:{}", *(token.value), token.span.line, token.span.col),
                    cerne::code_snippet(code_sv, token.span, std::format("`{}` is not a valid expression starter.", *(token.value))),
                    token.span
                );
            }
            errors++;
            return nullptr;
    }

    return nullptr;
}

std::unique_ptr<cerne::Node> cerne::ParseMachine::parse_infix(std::unique_ptr<cerne::Node> lhs) {
    // first get the operation token and its precedence
    auto& token = list[offset];
    auto precedence = get_score(token.type);
    auto op = token.type;
    
    // consume operator token
    offset++;

    // check if op is a suffix operator
    if(std::ranges::find(suffix, op) != suffix.end()) {
        return std::make_unique<cerne::SuffixExpr>(token.span, std::move(lhs), op);
    }
    
    // create rhs node for binary expression
    std::unique_ptr<Node> rhs = nullptr;

    // for right associative operators
    if(std::ranges::find(right_associative, op) != right_associative.end()) {
        // parse the right-hand side with lower precedence to get tokens of same precedence (like 2**3**4 = (2**(3**4))
        rhs = parse_expr(precedence - 1);
    } else {
        // for left-associative operators, parse the right-hand side with the same precedence
        rhs = parse_expr(precedence);
    }

    // range has its own node type since it differs in logic from other binary expressions
    if(token.type == TokenTypes::RANGE) {
        return std::make_unique<cerne::RangeExpr>(token.span, std::move(lhs), std::move(rhs));
    }

    // assignment operations have score of 2, so instead of creating a new map, we could simply see if precedence is 2 and create an AssignmentExpr node
    if(precedence == 2) {
        return std::make_unique<cerne::AssignmentExpr>(token.span, std::move(lhs), std::move(rhs), op);
    }
    
    // comparison operators have score of 11 and 10
    if(precedence == 11 || precedence == 10) {
        return std::make_unique<cerne::ComparisonExpr>(token.span, std::move(lhs), std::move(rhs), op);
    }
    
    return std::make_unique<cerne::BinaryExpr>(token.span, std::move(lhs), std::move(rhs), op);
}

/**
 * Uses pratt parsing to parse expressions
 * (stops at after the last token of the expression)
 */
std::unique_ptr<cerne::Node> cerne::ParseMachine::parse_expr(size_t precedence, std::unique_ptr<cerne::Node> lhs, bool quiet) {
    if(is_eof()) return nullptr;

    // now we call parse_nud() to parse the left side of the expression
    auto left = (lhs) ? std::move(lhs) : parse_nud(quiet);

    if(!left) {
        // if there's no left node, it means the current token is not a valid expression starter, so we return nullptr (since parse_nud() already generates the error if needed)
        return nullptr;
    }

    while(offset < list.size() && get_score(list[offset].type) > precedence) {
        left = parse_infix(std::move(left));
    }

    return left;
}