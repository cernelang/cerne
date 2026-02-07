/*
    Cerne Compiler - Utility to read a file from a given path

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#include "../include/utils.hpp"

std::string cerne::readf(const std::string& path) {
    std::string content;
    std::ifstream file(path);

    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            content += line + "\n";
        }
        file.close();
    } else {
        // error handling
        cerne::error("[cli]", std::format("This file ({}) does NOT exit.", path), "");
    }

    return content;
}