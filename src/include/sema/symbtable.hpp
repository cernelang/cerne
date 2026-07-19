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

        explicit Entry(
            const std::string& name, 
            TypeOfEntry entry_type, 
            Node* node = nullptr
        ) : name(name), entry_type(entry_type), node(node) {};
    };

    struct VariableEntry : Entry {
        Path* type;
        bool is_mutable;

        explicit VariableEntry(
            const std::string& name, 
            Path* type, 
            bool is_mutable, 
            Node* node = nullptr
        ) : Entry(name, TypeOfEntry::VARIABLE, node), type(type), is_mutable(is_mutable) {};
    };

    using entry_map = std::unordered_map<std::string, std::unique_ptr<Entry>>;

    struct FunctionEntry : Entry {
        Path* return_type;
        const std::vector<std::unique_ptr<Parameter>>* parameters;
        std::vector<std::unique_ptr<FunctionEntry>> overloads{};

        // all entries inside function scope
        entry_map entries{};

        explicit FunctionEntry(
            const std::string& name, 
            Path* return_type, 
            const std::vector<std::unique_ptr<Parameter>>* parameters, 
            Node* node = nullptr
        ) : Entry(name, TypeOfEntry::FUNCTION, node), return_type(return_type), parameters(parameters) {};
    };

    struct SEMAScope {
        // parent scope and parent entry (parent entry is important for differentiating between a function scope and a class scope for example)
        std::unique_ptr<SEMAScope> parent_scope = nullptr;
        std::unique_ptr<Entry> parent_entry = nullptr;

        // current scope entries
        entry_map entries;
    };

    class SymbolTable {
        // stack of scopes, where the first scope is the global scope
        std::vector<std::unique_ptr<SEMAScope>> scopes;

        public:
            AST* ast;
            const char* file_path;
            size_t errors = 0;
            std::string_view code_sv;

            explicit SymbolTable(AST* _ast, const char* file_path, const std::string_view& code_sv) : ast(_ast), file_path(file_path), code_sv(code_sv) {};
            ~SymbolTable()=default;

            void collect(const std::vector<std::unique_ptr<Node>>& node_list, SEMAScope* global_scope, FunctionEntry* current_function);
            void try_fun(entry_map* entries, const std::string& name, std::unique_ptr<FunctionEntry> fun_entry);
            void try_var(entry_map* entries, const std::string& name, std::unique_ptr<VariableEntry> var_entry);

            void build();
            void resolve();
    };
}

#endif