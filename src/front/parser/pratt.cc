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

std::unique_ptr<cerne::Node> cerne::ParseMachine::parse_nud() {
    auto& token = list[offset];
    auto type = token.type;

    switch (type) {
        // identifiers should call a LiteralExpr
        case TokenTypes::IDENTIFIER:
            offset++;
            return std::make_unique<cerne::LiteralExpr>(token.span, std::move(token.value));

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
            auto params = parse_expr(0);

            // after expression is parsed, check if the token it stopped at is a )
            const auto& current_token = list[offset];

            // groups should begin with ( and end with )
            if(current_token.type != TokenTypes::END_PARAM) {
                expected(
                    file_path,
                    token.span,
                    TokenTypeNames.at(TokenTypes::END_PARAM),
                    TokenTypeNames.at(current_token.type)
                );

                errors++;
            }

            offset++;

            return params;
        }

        default:
            break;
    }

    return nullptr;
}

std::unique_ptr<cerne::Node> cerne::ParseMachine::parse_infix(std::unique_ptr<cerne::Node> lhs) {
    // first get the operation token and its precedence
    auto& token = list[offset];
    auto precedence = get_score(token.type);
    auto op = token.type;

    offset++;
    
    std::unique_ptr<Node> rhs = nullptr;

    // for right associative operators
    if(std::find(right_associative.begin(), right_associative.end(), op) != right_associative.end()) {
        // parse the right-hand side with lower precedence to get tokens of same precedence (like 2**3**4 = (2**(3**4))
        rhs = parse_expr(precedence - 1);
    } else {
        // for left-associative operators, parse the right-hand side with the same precedencez
        rhs = parse_expr(precedence);
    }
    
    return std::make_unique<cerne::BinaryExpr>(token.span, std::move(lhs), std::move(rhs), op);
}

/**
 * Uses pratt parsing to parse expressions
 */
std::unique_ptr<cerne::Node> cerne::ParseMachine::parse_expr(size_t precedence) {
    // first token we parse, we check if the current token is a prefix or primary token (allowed to start an expression), if not, we generate an error and return
    const auto& token = list[offset];

    if( 
        token.type != TokenTypes::IDENTIFIER && 
        token.type != TokenTypes::NUMBER && 
        token.type != TokenTypes::STRING && 
        token.type != TokenTypes::FSTRING && 
        token.type != TokenTypes::SSTRING &&
        std::find(unary.begin(), unary.end(), token.type) == unary.end()
    ) {
        // generate an error and return
        cerror(
            file_path, 
            ERR_UNEXPECTED_TOKEN,
            std::format("Unexpected token `{}` at {}:{}", cerne::TokenTypeNames.at(token.type), token.span.line, token.span.col), 
            cerne::code_snippet(code_sv, token.span, std::format("`{}` cannot start an expression.", cerne::TokenTypeNames.at(token.type))),
            token.span
        );
        return nullptr;
    }

    // now we call parse_nud() to parse the left side of the expression
    auto left = parse_nud();

    if(left == nullptr) return nullptr;

    while(offset < list.size() && get_score(list[offset].type) > precedence) {
        left = parse_infix(std::move(left));
    }

    return left;
}