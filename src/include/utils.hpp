/*
    Cerne Compiler - Utils header

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#ifndef CE_UTILS
#define CE_UTILS

// std library
#include<stdlib.h>
#include<cstring>

#include<iostream>
#include<string>
#include<fstream>
#include<vector>
#include<map>
#include<functional>
#include<utility>
#include<algorithm>
#include<chrono>
#include<format>
#include<variant>

// util injection in cerne namespace
namespace cerne {
    typedef std::map<std::string, void*> args;
    typedef std::variant<std::function<void()>, std::function<void(char**, int)>> callback;

    std::string readf(const std::string&);
    args parse_args(int, char**);
    void error(const char*, const std::string&, const std::string&);
    void tlog(double, const std::string&);

    class CLI {
        private:
            args __args;

        public:
            // constructor and destructor
            CLI(const args&);
            ~CLI();

            // others
            void event(std::string, callback);
    };
}

#endif