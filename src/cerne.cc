/*
    Cerne Compiler - Main Entry Point

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#include "include/cerne.hpp"

int main(int argc, char** argv) {
    // for debugging later auto start = std::chrono::steady_clock::now();

    const cerne::args& args = cerne::parse_args(argc, argv);
    auto cli = std::make_unique<cerne::CLI>(args);

    cli->event("files", [&args](char** files, int file_size) {
        if(files != nullptr) {
            for(size_t i = 0; i < static_cast<size_t>(file_size); i++) {
                const char* file = files[i];
                std::string code = cerne::readf(std::string(file));
                const auto& tokens = cerne::lexer(std::string_view(code), file, args);
                std::cout << "Tokens: " << tokens.size() << std::endl;
                const auto& ast = cerne::parse(tokens, file, args);
            }
        }
    });

    cli->event("help", [&cli]() {
        cli->help();
    });

    cli->event("version", []() {
        std::cout << "Cerne is running on version: " << CERNE_VERSION << std::endl;
    });

    return 0;
}