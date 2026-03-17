/*
    Cerne Compiler - Utils header

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#ifndef CE_UTILS
#define CE_UTILS

#define ESC         "\x1b"
#define FG          ESC "[38;5;"
#define BG          ESC "[48;5;"
#define RESET       ESC "[0m"
#define SIGNATURE   "[cerne]"

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
#include<string_view>
#include<memory>
#include<cmath>

// error codes
#define ERR_UNEXPECTED_SYMBOL   1
#define ERR_TOO_MANY_DOTS       2
#define ERR_UNEXPECTED_TOKEN    3
#define ERR_UNKNOWN_KEYWORD     4

// util injection in cerne namespace
namespace cerne {
    
    // structs
    typedef struct Span {
        size_t line;
        size_t col;
        size_t offset;
        size_t length;
    } Span;
    

    // CLI arg structs
    typedef std::pair<std::string, std::string> flag;

    typedef struct args {
        std::map<std::string, std::string> flags;
        std::vector<std::string> files;
    } args;

    typedef std::variant<std::function<void()>, std::function<void(std::vector<std::string>)>> callback;
    

    // Diagnostics
    std::string_view code_snippet(const std::string_view& code, Span span, const std::string_view& under_message = "");
    void error(const char* where, const std::string& message);
    void cerror(const char* src, const size_t& errcode, const std::string_view& message, const std::string_view& code_snippet, const Span& span);
    void expected(const char* src, const Span& span, const std::string_view& expected, const std::string_view& got);
    void tlog(double time, const std::string& message);
    void debug(const std::string_view& message);

    // CLI tooling
    class CLI {
        private:
            args __args;

        public:
            // constructor and destructor
            CLI(const args& _args) : __args(_args) {};
            ~CLI()=default;

            // others
            void event(std::string name, callback cb);
            void help() const;
    };
    
    // misc utils
    std::string readf(const std::string& path);
    args parse_args(int argc, char** argv);
    std::string json(void* AST);
}

#endif