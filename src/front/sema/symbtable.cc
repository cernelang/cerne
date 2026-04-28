/*
    Cerne Compiler - The first step to SEMA, the symbol table, which is responsible for storing all symbols in the program, types, scopes and logically check if everything is in order regarding symbols
    
    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#include "../../include/sema/symbtable.hpp"



void cerne::SymbolTable::build() {
    const auto& node_list = ast->root->node_list;

    // create global scope
    auto global_scope = std::make_unique<SEMAScope>();
    scopes.push_back(std::move(global_scope));

    for(size_t i = 0; i < node_list.size(); i++) {
        const auto& node = node_list.at(i);

        switch(node->type) {
            case NodeType::FunNode: {
                const auto& fun_node = static_cast<FunNode*>(node.get());
                const auto& name = fun_node->name.data();
                const auto& base_entry = Entry{
                    name,
                    TypeOfEntry::FUNCTION,
                    fun_node
                };
                const auto& entry = FunctionEntry{
                    .base_data = base_entry,
                    .return_type = fun_node->return_type.get(),
                    .parameters = std::move(fun_node->parameters)
                };

                scopes.at(0)->entries[name] = std::make_unique<Entry>(base_entry);
                break;
            }

            default: {
                break;
            }
        }        
    }
}