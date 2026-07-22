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
#include<string_view>
#include<memory>
#include<cmath>
#include<thread>
#include<future>
#include<atomic>

// cerne
#include "errors.hpp"


// ansii escape codes for styling + signature
#define ESC         "\x1b"
#define FG          ESC "[38;5;"
#define BG          ESC "[48;5;"
#define RESET       ESC "[0m"
#define BOLD        ESC "[1m"
#define UNDERLINE   ESC "[4m"
#define SIGNATURE   "[cerne]"


// color palette
struct ce_colors {
    // foregrounds
    static constexpr const char* fglblue    = FG "159m";
    static constexpr const char* fgblue     = FG "81m";
    static constexpr const char* fgwhite    = FG "255m";
    static constexpr const char* fggray     = FG "248m";
    static constexpr const char* fggreen    = FG "120m";
    static constexpr const char* fgred      = FG "196m";
    static constexpr const char* fgpink     = FG "219m";
    static constexpr const char* fgyellow   = FG "220m";

    // backgrounds
    static constexpr const char* bgblack    = BG "0m";
    static constexpr const char* bgwhite    = BG "255m";
    static constexpr const char* bgpink     = BG "219m";
};


// cerne version update comes here for now
struct Version {
    int alpha;
    int major;
    int minor;
};

constexpr Version CERNE_VERSION{0, 3, 0};


// util injection in cerne namespace
namespace cerne {
    
    // structs
    struct Span {
        size_t line;
        size_t col;
        size_t offset;
        size_t length;
    };
    

    // CLI arg structs
    using flag = std::pair<std::string, std::string>;
    using callback = std::variant<std::function<void()>, std::function<void(std::vector<std::string>)>>;

    struct args {
        std::map<std::string, std::string> flags;
        std::vector<std::string> files;
    };
    

    // Diagnostics

    /**
     * Due to cerne's way of diagnostics, we need to store the errors (each error is a CERROR struct) in a vector and THEN print them all at once
     */
    struct CERROR {
        std::string src;
        size_t errcode;
        Span span;

        // has to be string since they need to own the object
        std::string message;
        std::string code_snippet;
        std::string extras;
    };

    std::string cerr_to_msg(const CERROR& error);

    class DEngine {
        std::vector<CERROR> errors;

        public:
            DEngine()=default;
            ~DEngine()=default;

            void add_error(const CERROR& error);
            void print_errors();
            static void print_errors(const std::vector<CERROR>& errors);
            std::vector<CERROR> get() { return errors; };
            void clear() { errors.clear(); };
    };

    class DebugEngine {
        std::vector<std::string> messages;

        public:
            DebugEngine()=default;
            ~DebugEngine()=default;

            void add_message(const std::string& message);
            static void print_messages(const std::vector<std::string>& messages);
            std::vector<std::string> get() { return messages; };
            void clear() { messages.clear(); };
    };

    // this is actually really cool because you need extern for this case since you need other files to access this variable that is initialized in the compilation loop and thread_local for thread-safety reasons
    extern thread_local DEngine* diag_engine;

    // same for debug
    extern thread_local DebugEngine* debug_engine;

    // helpers
    std::string code_snippet(const std::string_view& code, Span span, const std::string_view& under_message = "");
    std::string oneline_code_snippet(const std::string_view& code, Span span);
    void error(const char* where, const std::string& message);
    void cerror(const char* src, const size_t& errcode, const std::string_view& message, const std::string_view& code_snippet, const Span& span, const std::string_view& extras = "");
    void tlog(double time, const std::string& message);
    void debug(const std::string_view& message);
    std::string help(const std::string& base_note);

    // CLI tooling
    class CLI {
        private:
            args __args;

        public:
            // constructor and destructor
            explicit CLI(const args& _args) : __args(_args) {};
            ~CLI()=default;

            // others
            void event(const std::string& name, callback cb) const;
            void help() const;
    };

    // JSON Building

    // set for styling purposes
    constexpr size_t JSON_INDENT_SIZE = 4;

    struct JSON {
        std::map<std::string, std::variant<std::string, JSON, bool, size_t>> properties;
    };

    class JSONBuilder {
        public:
            JSON json;
            size_t indentation_level;

            explicit JSONBuilder(JSON json = {}, size_t indentation_level = 0) : json(std::move(json)), indentation_level(indentation_level) {}
            ~JSONBuilder()=default;

            std::string build();
            std::string convert_array(const std::variant<std::vector<std::string>, std::vector<JSON>, std::vector<bool>, std::vector<size_t>>& arr);
            void add_property(const std::string& key, const std::variant<std::string, JSON, bool, size_t>& value);
    };
    
    // misc utils
    std::string readf(const std::string& path);
    args parse_args(int argc, char** argv);
    
    // forward declaration because of circular dependencies
    class AST;
    std::string json(const AST* ast, bool no_ast);
}

#endif