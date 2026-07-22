/*
    Cerne Compiler - Import node blueprint

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#include "../../../include/parser/handler.hpp"

// reduce verbosity
using enum cerne::TokenTypes;

/**
 * Simple utility to follow the import path
 */
void follow_import_path(const cerne::blueprint_arguments& args, std::unique_ptr<cerne::ImportNode>& current_import) {
    const auto& machine = args.machine;
    std::vector<std::string> path;
    std::vector<cerne::Span> path_spans;

    if(!machine->expect(IDENTIFIER)) return;

    while(machine->offset < machine->list.size()) {
        if(!machine->expect_or({IDENTIFIER, DOT}, true)) break;

        const auto& current = machine->peek();

        // we simply push everytime there's an identifier, if there's a dot we just keep going
        if(current.type == IDENTIFIER) {
            path.push_back(*(current.value));
            path_spans.push_back(current.span);
        } else {
            machine->advance();
            if(!machine->expect(IDENTIFIER)) return; // if after the dot there's no identifier, then it's an error and we return an empty path

            // else we push the identifier after the dot and keep going
            const auto& nextafterdot = machine->peek();
            path.push_back(*(nextafterdot.value));
            path_spans.push_back(nextafterdot.span);
        }

        machine->advance();
    }

    current_import->package_path = path;
    current_import->package_path_spans = path_spans;

    return;
}

/**
 * the import syntax is quite straightforward with just a few rules, however, these rules change DRASTICALLY what to import, namely:
 * import "path/to/file"        -> imports a file
 * import mypackage             -> imports a package, not a file
 * import mypackage.submodule   -> imports a submodule of a package
 * import std.<something>       -> imports a standard library module 
 * import user::package         -> imports a package specific to a user
 * and these can somewhat compound (like user::package.submodule) but the rules stay the same.
 */
std::unique_ptr<cerne::Node> cerne::Import(const blueprint_arguments& args) {
    const auto& machine = args.machine;

    // immediately check if the first token is a string or identifier (only acceptable first tokens)
    machine->advance();

    if(!machine->expect_or({ 
        IDENTIFIER, 
        STRING, 
        SSTRING, 
        FSTRING 
    })) return nullptr;

    // already create an import node
    const auto& current_token = machine->peek();
    auto current_import = std::make_unique<cerne::ImportNode>(current_token.span);

    // if it's a string, then it's a file path import, in which we can simply set the file path and return
    if(
        current_token.type == STRING ||
        current_token.type == SSTRING ||
        current_token.type == FSTRING
    ) {
        current_import->file_path = *(current_token.value);
        current_import->file_path_span = current_token.span;
    } else {
        /*
            now if it's an identifier things get slightly more complicated since it can be a user or a package
            to distinguish though it's easy, we simply peek the next token and if it's a dot, then it's a package
            otherwise if it's a member access (::) then it's a user
            (if it's neither that, nor an END token, then it's an error since the import statement has been malformed)
        */ 
        current_import->is_path = false; 
        
        const auto& next = machine->peek(1);

        if (next.type == DOT) {
            // it's a package
            follow_import_path(args, current_import); // in this case, import_path is GUARANTEED to have at least 1 member (the first identifier), so no need to worry about error checking
            current_import->is_package = true;
        } else if(next.type == MEMBER_ACCESS) {
            // it's a user

            // advance the member access token to get the package path
            machine->advance(2); 

            // here this is volatile, since the package path can be empty (like user::, in which there is no package specified)
            follow_import_path(args, current_import); 

            // we can just return nullptr since skip_to_next_end is handled by follow_import_path's error handling
            if(current_import->package_path.empty()) return nullptr;

            // set other user import properties
            current_import->is_from_user = true;
            current_import->user = *(current_token.value);
            current_import->user_span = current_token.span;
        } else if(next.type == END) {
            // regular identifier import means it's just a single package import with nothing special about it, so we just set the package path to the identifier and set the is_package flag to true
            current_import->is_package = true;
            current_import->package_path.push_back(*(current_token.value));
            current_import->package_path_spans.push_back(current_token.span);
        } else {
            // malformed error
            cerne::cerror(
                machine->file_path,
                ERR_MALFORMED_IMPORT,
                "Malformed import statement",
                cerne::code_snippet(
                    machine->code_sv,
                    next.span,
                    std::format("Unexpected '{}' after '{}' in the import statement", *(next.value), *(current_token.value))
                ),
                next.span
                // cerne::help()
            );
            machine->skip_to_next_end();
            machine->errors++;
            return nullptr;
        }
    }

    return current_import;
}