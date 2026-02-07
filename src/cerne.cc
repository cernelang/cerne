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
    cerne::CLI* cli = new cerne::CLI(args);

    cli->event("files", [&](char** files, int file_size) {
        if(files != nullptr) {
            for(size_t i = 0; i < static_cast<size_t>(file_size); i++) {
                const char* file = files[i];
                std::string code = cerne::readf(std::string(file));
                std::cout << code << std::endl;
            }
        }
    });

    cli->event("help", [&]() {
        std::cout << "Cerne help message soon" << std::endl;
    });

    cli->event("version", [&]() {
        std::cout << "Cerne is running on version: " << CERNE_VERSION << std::endl;
    });

    delete cli;
    return 0;
}