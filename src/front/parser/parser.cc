/*
    Cerne Compiler - Parser Component, responsible for converting the token list from the previous lexer component to a more comprehensible node tree (AST).

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#include "../include/parser.hpp"
#include "../include/parser/handler.hpp"

const std::map<std::string, std::function<std::unique_ptr<cerne::Node>(const cerne::blueprint_arguments&)>> blueprints = {
    { "return", static_cast<std::unique_ptr<cerne::Node>(*)(const cerne::blueprint_arguments&)>(cerne::Return) },
    { "fun", static_cast<std::unique_ptr<cerne::Node>(*)(const cerne::blueprint_arguments&)>(cerne::Fun) },
    { "let", static_cast<std::unique_ptr<cerne::Node>(*)(const cerne::blueprint_arguments&)>(cerne::Let) },
    { "const", static_cast<std::unique_ptr<cerne::Node>(*)(const cerne::blueprint_arguments&)>(cerne::_Const) },
    { "import", static_cast<std::unique_ptr<cerne::Node>(*)(const cerne::blueprint_arguments&)>(cerne::Import) },
    { "export", static_cast<std::unique_ptr<cerne::Node>(*)(const cerne::blueprint_arguments&)>(cerne::Export) }
};

/**
 * Execute mnemonic blueprint
 */
std::unique_ptr<cerne::Node> cerne::ParseMachine::parse_mnemonic() {
    const auto& token = list[offset];
    std::string token_name = std::string(token.value.get()->c_str());
    if(blueprints.contains(token_name)) {
        auto arguments = cerne::blueprint_arguments{
            .machine=this
        };

        return blueprints.at(token_name)(arguments);
    } else {
        cerne::cerror(
            file_path, 
            ERR_UNKNOWN_KEYWORD,
            std::format("Unknown keyword `{}` at {}:{}", token_name, token.span.line, token.span.col), 
            cerne::code_snippet(code_sv, token.span, std::format("`{}` does not exist yet.", *(token.value))),    
            token.span
        );
        errors++;
    }
    return nullptr;
}


/**
 *  separate function for the identifier case
 */
std::unique_ptr<cerne::Node> identifier_case(cerne::ParseMachine* machine) {
    auto path = machine->parse_path();
    
    // peek next token
    auto& next_token = machine->peek();

    // variable declaration
    if(next_token.type == cerne::TokenTypes::IDENTIFIER) {
        auto& equals = machine->peek(1);

        // if there isn't an equals, it's an uninitialized variable declaration
        if(equals.type != cerne::TokenTypes::EQU) {
            return std::make_unique<cerne::VarDecl>(
                path->span, 
                *(next_token.value), 
                next_token.span,
                false, 
                true, 
                std::move(path), 
                nullptr
            );
        }

        // initialized variable declaration

        machine->advance(2); // advance past the variable name and the equals sign

        // now the value should be an expression, so we parse it
        auto value = machine->parse_expr(0);

        return std::make_unique<cerne::VarDecl>(
            path->span, 
            *(next_token.value), 
            next_token.span,
            false, 
            false, 
            std::move(path), 
            std::move(value)
        );
    } else {
        // default to parse_expr (we need to make first literalexpr to be the nud of the expression)
        return machine->parse_expr(0, std::make_unique<cerne::LiteralExpr>(path->span, std::move(path)));
    }
}


/**
 * Global parse function, executes appropriate function depending on the current token
 */
std::unique_ptr<cerne::Node> cerne::ParseMachine::parse(cerne::Token& token) {
    switch(token.type) {
        case TokenTypes::MNEMONIC: {
            return parse_mnemonic();
        }

        case TokenTypes::START_SCOPE: {
            return parse_scope();
        }

        /**
         * identifier starters can be lots of things, for instance:
         * - the start of a variable declaration (int x = 10)
         * - an execution expression (my_function())
         * - the start of an assignment expression (x += 5)
         */
        case TokenTypes::IDENTIFIER: {
            return identifier_case(this);
        }

        case TokenTypes::STRING:
        case TokenTypes::FSTRING:
        case TokenTypes::SSTRING:
        case TokenTypes::NUMBER: {
            // parse expressions
            return parse_expr(0);
        }

        default:
            // not valid
            break;
    }

    return nullptr;
}

void cerne::ParseMachine::walk() {
    for(; offset < list.size(); offset++) {
        auto& token = list[offset];
        auto node = parse(token);
        if(node) ast->root->node_list.push_back(std::move(node));
    }
}

/* --- Parse Machine Methods Over --- */

/* --- main parse function --- */

std::unique_ptr<cerne::AST> cerne::parse(const std::string_view& code_sv, cerne::tokenlist& list, const char* path, const cerne::args& options) {
    // initialize the AST and the machine
    auto ast = std::make_unique<AST>(path);
    auto machine = std::make_unique<ParseMachine>(ast.get(), list, path, options, code_sv);

    // begin machine walk
    machine->walk();

    // before returning, pass onto the AST the errors/warnings (important for the CLI's main build loop)
    ast->errors     =   machine->errors;
    ast->warnings   =   machine->warnings;

    return ast;
}