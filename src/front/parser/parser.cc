/*
    Cerne Compiler - Parser Component, responsible for converting the token list from the previous lexer component to a more comprehensible node tree (AST).

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#include "../include/parser.hpp"
#include "blueprints/handler.hpp"

class ParseMachine {
    public:
        std::shared_ptr<cerne::AST> ast;

        ParseMachine(std::shared_ptr<cerne::AST> _ast) : ast(_ast) {};
        ~ParseMachine()=default;
};

std::map<std::string, std::function<size_t(const cerne::blueprint_arguments&)>> blueprints = {
    { "return", static_cast<size_t(*)(const cerne::blueprint_arguments&)>(cerne::Return) },
    { "fun", static_cast<size_t(*)(const cerne::blueprint_arguments&)>(cerne::Fun) }
};

std::shared_ptr<cerne::AST> cerne::parse(const cerne::tokenlist& list, const char* path, const cerne::args& options) {
    auto ast = std::make_shared<AST>();
    auto machine = new ParseMachine(ast);

    for(size_t i = 0; i < list.size(); i++) {
        const auto& token = list[i];

        switch(token.type) {
            
            case TokenTypes::MNEMONIC: {
                std::string token_name = std::string(token.value.get()->c_str());
                if(blueprints.find(token_name) != blueprints.end()) {
                    auto arguments = cerne::blueprint_arguments{
                        .ast=ast,
                        .list=list,
                        .offset=i
                    };

                    size_t surplus = blueprints[token_name](arguments);
                    i += surplus;
                }
                break;
            }

            default:
                //
                break;
        }
    }

    delete machine;
    return ast;
}