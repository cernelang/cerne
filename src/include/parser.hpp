/*
    Cerne Compiler - Parser Component, responsible for converting the token list from the previous lexer component to a more comprehensible node tree (AST).

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#ifndef CE_PARSER
#define CE_PARSER

#include "utils.hpp"
#include "lexer.hpp"

namespace cerne {
    enum class Types {
        // Defaults
        Void, Auto, Any,

        // Literals
        I8, I16, I32, I64,
        U8, U16, U32, U64,
        Char, Str, List, 
        
        // Concurrency
        Signal,

        // Misc
        Object, Function, Space
    };

    typedef struct Symbol {
        std::string_view name;
        size_t scope;
        Types type;
    } Symbol;

    enum class NodeTypes {
        PROGRAM,
        STATEMENT,
        EXPRESSION,
        LITERAL,
        FUNC_CALL,
        FUNC_DECL,
        VAR_DECL,
        IF, ELIF, ELSE,
        LOOP,
        SPACE_DECL,
        OBJECT_DECL
    };
    
    class Node {
        protected:
            NodeTypes type;

        public:
            // node can be initialized with a nodetype or not
            Node()=default;
            Node(NodeTypes t) : type(t) {};

            // default destructor
            virtual ~Node()=default;
    };

    class BinaryExpr : public Node {
        private:
            // left hand-side node & right hand-side node
            std::unique_ptr<Node> lhs;
            std::unique_ptr<Node> rhs;

        public:
            BinaryExpr(Node lhs, Node rhs) : lhs(std::make_unique<Node>(lhs)), rhs(std::make_unique<Node>(rhs)) {};
            ~BinaryExpr()=default;
    };

    class Program : public Node {
        private:
            // body of nodes
            std::vector<std::unique_ptr<Node>> node_list;

        public:
            Program(): Node(NodeTypes::PROGRAM) {};
            ~Program()=default;
    };
    
    // ID [u64] : Symbol
    typedef std::map<size_t, Symbol> symbol_table;

    class AST {
        private:
            // stores the actual
            std::unique_ptr<Program> root;
            
            // for Diag
            const char* file_path;
        public: 
            AST()=default;
            ~AST()=default;
    };

    std::shared_ptr<AST> parse(const tokenlist& list, const char* file_path, const args& options);
}

#endif