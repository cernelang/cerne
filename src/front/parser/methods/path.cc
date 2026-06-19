/*
    Cerne Compiler - Component of the parser. Responsible for parsing path elements in types, expressions and calls.

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#include "../../../include/parser.hpp"

using enum cerne::TokenTypes;

void push_path_element(
    std::unique_ptr<cerne::Path>& path, 
    bool* strict,
    std::string name,
    bool is_member = false,
    std::unique_ptr<cerne::CallData> call = nullptr,
    std::unique_ptr<cerne::InitializerData> initializer = nullptr,
    std::vector<std::unique_ptr<cerne::Path>> generic_args = {}
) {
    if(call || initializer) {
        *strict = true; // if we encounter a call or an initializer, we know for sure this is not a pure path (aka a type path)
    }

    path->basic_path.push_back(cerne::BasicPathElement{
        .name = std::move(name),
        .is_member = is_member,
        .call = (call ? std::move(call) : nullptr),
        .initializer = (initializer ? std::move(initializer) : nullptr),
        .generic_args = (generic_args.empty() ? std::vector<std::unique_ptr<cerne::Path>>() : std::move(generic_args))
    });
}

/**
 * parses a path
 * strict makes it so it can ONLY be a pure path (aka a type path)
 * this means parse_path will stop parsing as soon as it encounters a call or an initializer
 */
std::unique_ptr<cerne::Path> cerne::ParseMachine::parse_path(bool strict) {
    auto path = std::make_unique<cerne::Path>();
    path->pure_path = true; // right off the bat we assume it is a pure path (aka a type path)

    // begin the path loop
    bool should_break = false;

    // current element info
    std::string current_name;

    
    while(!should_break && offset < list.size()) {
        const auto& token = peek();
        const auto& token_type = token.type;

        switch(token_type) {
            case IDENTIFIER: {
                current_name = *token.value;
                break;
            }

            case START_PARAM: {
                
                break;
            }

            default: {
                should_break = true;
                continue; // we break out of the loop but we don't want to skip the token since it might be important for the next part of the parsing
            }
        }

        advance();
    }

    return path;
}