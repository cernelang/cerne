/*
    Cerne Compiler - Main Entry Point

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#include "include/cerne.hpp"

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
        const auto& tokens = cerne::lexer(code_sv, file, args);
        if(tokens.size() == 0) break;
        if(args.flags.find("debug") != args.flags.end()) {
            cerne::debug(std::format("Tokens -> {}", tokens.size()));
        }

        // after lexing, we pass the token list through the parser to generate an AST
        const auto& ast = cerne::parse(code_sv, tokens, file, args);

        // after parsing, we pass the AST through SEMA and then to IR generation, for now though, since those haven't been developed yet, the if statement will be blank
        if(ast->errors > 0) {
            cerne::error(file, std::format("Compilation failed with {}{}{} error{}", FG "196m", ast->errors, FG "255m", ((ast->errors >= 2)?"s!":"!")));
            break;
        }
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
        std::cout << "Cerne is running on version: " << CERNE_VERSION << std::endl;
    });

    return 0;
}