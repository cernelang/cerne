/*
    Cerne Compiler - Parser Component, responsible for converting the token list from the previous lexer component to a more comprehensible node tree (AST).

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#include "../include/parser.hpp"
#include "blueprints/handler.hpp"

const std::map<std::string, std::function<size_t(const cerne::blueprint_arguments&)>> blueprints = {
    { "return", static_cast<size_t(*)(const cerne::blueprint_arguments&)>(cerne::Return) },
    { "fun", static_cast<size_t(*)(const cerne::blueprint_arguments&)>(cerne::Fun) }
};

std::unique_ptr<cerne::AST> cerne::parse(cerne::tokenlist list, const char* path, const cerne::args& options) {
    auto ast = std::make_unique<AST>();

    for(size_t i = 0; i < list.size(); i++) {
        const auto& token = list[i];

        switch(token.type) {
            
            case TokenTypes::MNEMONIC:
                
                break;
            
            default:
                //
                break;
        }
    }

    return ast;
}