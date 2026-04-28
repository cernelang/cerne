/*
    Cerne Compiler - component of the SEMA stage, responsible for constructing a symbol table

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#ifndef CE_SEMA_SYMBTABLE
#define CE_SEMA_SYMBTABLE

#include "../utils.hpp"
#include "../lexer.hpp"
#include "../parser.hpp"
#include "../parser/types.hpp"

namespace cerne {
    enum class TypeOfEntry {
        VARIABLE,
        FUNCTION,
        SPACE,
        MEMBER,
        PROPERTY,
        PARAMETER,
        CLASS,
        STRUCT,
        ENUM,
        TEMPLATE
    };

    struct Entry {
        std::string name;
        TypeOfEntry entry_type;
        
        // for diagnostics
        Node* node;
    };

    struct VariableEntry {
        Entry base_data;
        Type* type;
        bool is_mutable;
    };

    struct FunctionEntry {
        Entry base_data;
        Type* return_type;
        std::vector<std::unique_ptr<Parameter>> parameters;
        std::vector<std::unique_ptr<FunctionEntry>> overloads = {};
    };

    struct SEMAScope {
        // parent scope and parent entry (parent entry is important for differentiating between a function scope and a class scope for example)
        std::unique_ptr<SEMAScope> parent_scope = nullptr;
        std::unique_ptr<Entry> parent_entry = nullptr;

        // current scope entries
        std::unordered_map<std::string, std::unique_ptr<Entry>> entries;
    };

    class SymbolTable {
        std::unique_ptr<AST> ast;
        // stack of scopes, where the first scope is the global scope
        std::vector<std::unique_ptr<SEMAScope>> scopes;

        public:
            explicit SymbolTable(std::unique_ptr<AST> _ast) : ast(std::move(_ast)) {};
            ~SymbolTable()=default;

            void build();
            static SEMAScope make_scope(std::vector<std::unique_ptr<Node>> node_list, SEMAScope* parent_scope = nullptr, Entry* parent_entry = nullptr);
    };
}

#endif