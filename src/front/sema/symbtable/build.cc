/*
    Cerne Compiler - The first step to SEMA, the symbol table, which is responsible for storing all symbols in the program, types, scopes and logically check if everything is in order regarding symbols
    
    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#include "../../../include/sema/symbtable.hpp"

/**
 * check if name already exists in scope, if it does, check if that overload already exists, if not, push it on the existing functionentry's overloads vector, if it does just throw an error
 */
void cerne::SymbolTable::try_fun(entry_map* entries, const std::string& name, std::unique_ptr<FunctionEntry> fun_entry) {

    // function name exists, check signature
    if(const auto it = entries->find(name); it != entries->end()) {
        const auto entry = static_cast<FunctionEntry*>(it->second.get());
        const auto& overloads = entry->overloads;

        // check if there's a duplicate
        const auto duplicate_exists = std::ranges::any_of(overloads, [&fun_entry](const auto& overload) {
            const auto& possible_parameters = overload->parameters;
            const auto& fun_parameters = fun_entry->parameters;

            // deref and check for equality within each parameter
            return std::ranges::equal(*possible_parameters, *fun_parameters, [](const std::unique_ptr<cerne::Parameter>& a, const std::unique_ptr<cerne::Parameter>& b) {
                return *a == *b;
            });
        });

        if(duplicate_exists) {
            cerne::cerror(
                file_path,
                ERR_REDECLARED_FUNCTION,
                std::format("Function '{}' is already defined in this scope", name),
                cerne::code_snippet(cerne::readf(file_path), fun_entry->node->span, "Function redefined here"),
                fun_entry->node->span,
                cerne::help(
                    std::format(
                        "definition for '{}' found here\n{}", name,
                        cerne::oneline_code_snippet(code_sv, entry->node->span)
                    )
                )
            );
            ast->errors++;
            return;
        }
    }

    entries->try_emplace(name, std::move(fun_entry));
}

void cerne::SymbolTable::try_var(entry_map* entries, const std::string& name, std::unique_ptr<VariableEntry> var_entry) {
    if(entries->contains(name)) {
        auto prev_entry = static_cast<VarDecl*>(entries->at(name)->node);
        auto var_node = static_cast<VarDecl*>(var_entry->node);

        cerne::cerror(
            file_path,
            ERR_REDECLARED_VARIABLE,
            std::format("Variable '{}' is already defined in this scope", name),
            cerne::code_snippet(cerne::readf(file_path), var_node->name_span, "Variable redefined here"),
            var_node->name_span,
            cerne::help(
                std::format(
                    "definition for '{}' found here\n{}", name,
                    cerne::oneline_code_snippet(code_sv, prev_entry->name_span)
                )
            )
        );
        ast->errors++;
        return;
    }

    entries->try_emplace(name, std::move(var_entry));
}

/**
 * for now, only collect functions and variables (both in global scope and in function scopes)
 */
void cerne::SymbolTable::collect(const std::vector<std::unique_ptr<cerne::Node>>& node_list, cerne::SEMAScope* global_scope, cerne::FunctionEntry* current_function) {
    for(const auto& node : node_list) {
        const auto node_ptr = node.get();
        
        switch(node->type) {
            case NodeType::FunNode: {
                const auto& fun_node = static_cast<FunNode*>(node_ptr);
                const auto name = fun_node->name;
                
                auto function_entry = std::make_unique<FunctionEntry>(
                    name,
                    fun_node->return_type.get(),
                    &(fun_node->parameters),
                    fun_node
                );

                // collect inner declarations
                collect(fun_node->body->body, global_scope, function_entry.get());

                // global scope function
                if(!current_function) {
                    try_fun(&global_scope->entries, name, std::move(function_entry));
                    continue;
                }

                // local function
                try_fun(&current_function->entries, name, std::move(function_entry));
                break;
            }

            case NodeType::VarDecl: {
                const auto& var_node = static_cast<VarDecl*>(node_ptr);
                auto name = var_node->name;

                auto vardecl_entry = std::make_unique<VariableEntry>(
                    name,
                    var_node->var_type.get(),
                    !(var_node->is_const),
                    var_node
                );

                // global scope
                if(!current_function) {
                    try_var(&global_scope->entries, name, std::move(vardecl_entry));
                    continue;
                }

                // local scope
                try_var(&current_function->entries, name, std::move(vardecl_entry));
                break;
            }

            // for now the rest of the nodes are going to "break"
            default: {
                break;
            }
        }
    }
}

void cerne::SymbolTable::build() {
    const auto& node_list = ast->root->node_list;

    // create global scope
    auto global_scope = std::make_unique<SEMAScope>();

    // collect all declarations
    collect(node_list, global_scope.get(), nullptr);

    scopes.push_back(std::move(global_scope));

    // resolve them
    resolve();
}