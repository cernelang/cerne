/*
    Cerne Compiler - component of the parser, holds the AST class and nodes

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#ifndef CE_PARSER_AST
#define CE_PARSER_AST

#include "../utils.hpp"
#include "../lexer.hpp"
#include "types.hpp"
#include "node.hpp"

namespace cerne {
    // SUB NODES BEGIN HERE

    /**
     * Leafs are used for expressions
     * they hold the value of a literal such as a number or a string
     */
    struct Leaf : Node {
        std::unique_ptr<std::string> value;
        bool is_number;

        explicit Leaf(
            Span s, 
            std::unique_ptr<std::string> v, bool is_num = false
        ) : Node(NodeType::Leaf, s), value(std::move(v)), is_number(is_num) {};

        JSON to_json() override;
    };

    /**
     * LiteralExpr are also used for expressions
     * despire the name, they simply store the specific identifiers
     */
    struct LiteralExpr : Node {
        std::unique_ptr<std::string> value;

        explicit LiteralExpr(
            Span s, 
            std::unique_ptr<std::string> value
        ) : Node(NodeType::LiteralExpr, s), value(std::move(value)) {};

        JSON to_json() override;
    };

    /**
     * a + b
     */
    struct BinaryExpr : Node {
        // left hand-side node & right hand-side
        std::unique_ptr<Node> lhs;
        std::unique_ptr<Node> rhs;
        
        // we can know what the operation is by the tokentype alone, no need for another enum class
        TokenTypes op;

        // constructor for binary expression
        explicit BinaryExpr(
            Span s,
            std::unique_ptr<Node> lhs,
            std::unique_ptr<Node> rhs,
            TokenTypes op
        ) : Node(NodeType::BinaryExpr, s), lhs(std::move(lhs)), rhs(std::move(rhs)), op(op) {};
    
        JSON to_json() override;
    };

    // x: int
    struct Parameter : Node {
        // <> (get all remaining parameters in front of it)
        bool unpack;

        // name of the variable in the parameter
        std::string name;

        // type of the parameter(s)
        std::unique_ptr<Path> ptype;

        // constructor for the parameter node
        explicit Parameter(
            Span s,
            bool u = false,
            const std::string& name = ""
        ) : Node(NodeType::Parameter, s), unpack(u), name(name) {};
        
        JSON to_json() override;
    };
    
    // {Scope}
    struct Scope : Node {
        // the body of a scope is just a bunch of nodes, kinda like a program node
        std::vector<std::unique_ptr<Node>> body;

        // constructor for the scope node, with a default (empty) span
        explicit Scope(const Span& s = {}) : Node(NodeType::Scope, s) {};
        
        JSON to_json() override;
    };

    /**
     * return <expr>, <expr>, ... <end>
     */
    struct ReturnStmt : Node {
        std::vector<std::unique_ptr<Node>> values;

        explicit ReturnStmt(const Span& s) : Node(NodeType::ReturnStmt, s) {};

        JSON to_json() override;
    };

    // function definitions require a body, parameters, a return type and a name. every single one.
    struct FunNode : Node {
        // () {}
        std::vector<std::unique_ptr<Parameter>> parameters;
        std::unique_ptr<Scope> body;
        
        // void
        std::unique_ptr<Path> return_type;

        // identifier
        std::string name;

        // constructor for the function node (as in function definition, not declaration nor call)
        explicit FunNode(
            const Span& span,
            std::vector<std::unique_ptr<Parameter>> parameters, 
            std::unique_ptr<Scope> body, 
            std::unique_ptr<Path> return_type, 
            const std::string& name = ""
        ) : Node(NodeType::FunNode, span), parameters(std::move(parameters)), body(std::move(body)), return_type(std::move(return_type)), name(name) {};
        
        JSON to_json() override;
    };

    // variable declaration
    struct VarDecl : Node {
        std::string name;
        bool is_const;
        bool uninitialized;
        std::unique_ptr<Path> var_type;
        std::unique_ptr<Node> value;

        explicit VarDecl(
            const Span& span,
            const std::string& name = "",
            bool is_const = false,
            bool uninitialized = false,
            std::unique_ptr<Path> var_type = nullptr,
            std::unique_ptr<Node> value = nullptr
        ) : Node(NodeType::VarDecl, span), name(name), is_const(is_const), uninitialized(uninitialized), var_type(std::move(var_type)), value(std::move(value)) {};

        JSON to_json() override;
    };

    // import and export nodes

    struct ImportNode : Node {
        std::string file_path;
        std::string user;
        std::vector<std::string> package_path;
        bool is_path = true;
        bool is_package = false;
        bool is_from_user = false;

        explicit ImportNode(
            const Span& span, 
            const std::string& file_path = "", 
            const std::string& user = ""
        ) : Node(NodeType::Import, span), file_path(file_path), user(user) {};

        JSON to_json() override;
    };

    struct ExportNode : Node {
        std::string symbol;

        explicit ExportNode(
            const Span& span, 
            const std::string& symbol = ""
        ) : Node(NodeType::Export, span), symbol(symbol) {};

        JSON to_json() override;
    };

    // global node
    struct Program : Node {
        std::vector<std::unique_ptr<Node>> node_list;

        // constructor for the program node, no need for a span since it's just a container for the whole program
        Program() : Node(NodeType::Program, Span{0,0,0,0}) {};

        JSON to_json() override {
            auto json = std::make_unique<JSONBuilder>();
            
            json->convert_array(
                std::vector<JSON>(
                    node_list.size(),
                    JSON()
                )
            );
            
            return json->json;
        };
    };

    /**
     * AST class responsible for containing the root of the program tree
     * and also for storing the number of errors and warnings
     * (file path is also stored for diagnostics)
     */
    class AST {
        public: 
            // for proceeding
            size_t errors;
            size_t warnings;
            
            // stores the actual root of the AST
            std::unique_ptr<Program> root;
            
            // for Diag
            const char* file_path;

            explicit AST(const char* path): file_path(path) {
                root = std::make_unique<Program>();
                errors = 0;
                warnings = 0;
            };
            ~AST()=default;
    };
}

#endif