/*
    Cerne Compiler - Main Entry Point

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#include "include/cerne.hpp"

/**
 * spearate utility to check ast print conditions
 */
void check_ast_print(const cerne::args& args, const cerne::AST* ast, bool error = false) {
    // check for print/dump flags and their values (to see if it's exclusively for AST)
    if(args.flags.contains("print_ast") || (args.flags.contains("print") && args.flags.at("print") == "ast")) {
        std::cout << cerne::json(ast, error) << std::endl;
    } else if(args.flags.contains("dump_ast") || (args.flags.contains("dump") && args.flags.at("dump") == "ast")) {
        std::string dump_path = std::string(ast->file_path) + ".ast.json";
        std::ofstream dump_file(dump_path);
        dump_file << cerne::json(ast, error);
        dump_file.close();

        cerne::debug(std::format("AST dumped to {}", dump_path));
    }
}

/**
 * Compilation pipeline
 */
void compile_files(const cerne::args& args, std::vector<std::string> files) {
    for(size_t i = 0; i < files.size(); i++) {
        // first, we read the file's content
        const char* file = files[i].c_str();
        const auto& code = cerne::readf(std::string(file));
        const auto& code_sv = std::string_view(code);
        
        // now we pass it through the lexer
        auto tokens = cerne::lexer(code_sv, file, args);
        if(tokens.size() == 0) break;
        if(args.flags.contains("debug")) {
            cerne::debug(std::format("Tokens -> {}", tokens.size()));
        }

        // after lexing, we pass the token list through the parser to generate an AST
        const auto& ast = cerne::parse(code_sv, tokens, file, args);

        // after parsing, we pass the AST through SEMA and then to IR generation, for now though, since those haven't been developed yet, the if statement will be blank
        if(ast->errors > 0) {
            check_ast_print(args, ast.get(), true);
            cerne::error(file, std::format("Compilation failed with {}{}{} error{}", FG "196m", ast->errors, FG "255m", ((ast->errors >= 2)?"s!":"!")));
            break;
        }
        
        // add amount of nodes to debug information
        if(args.flags.contains("debug")) {
            cerne::debug(std::format("Nodes -> {}", ast->root->node_list.size()));
        }

        // ast diagnostics
        check_ast_print(args, ast.get());
    }
}

int main(int argc, char** argv) {
    // for debugging later auto start = std::chrono::steady_clock::now();
    const auto& args = cerne::parse_args(argc, argv);
    auto cli = std::make_unique<cerne::CLI>(args);

    cli->event("files", [&args](std::vector<std::string> files) {
        compile_files(args, files);
    });

    cli->event("help", [&cli]() {
        cli->help();
    });

    cli->event("version", []() {
        std::cout << "Cerne is running on version: v" << CERNE_VERSION.alpha << "." << CERNE_VERSION.major << "." << CERNE_VERSION.minor << std::endl;
    });

    return 0;
}