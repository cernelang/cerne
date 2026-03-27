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

namespace cerne {
    // now we start declaring what a node even is
    enum class NodeType {
        Leaf,
        LiteralExpr,
        BinaryExpr,
        Parameter,
        Scope,
        ReturnStmt,
        FunNode,
        VarDecl,
        Program
    };

    // nodetypenames
    const std::map<NodeType, std::string> NodeTypeNames = {
        {NodeType::Leaf, "Leaf"},
        {NodeType::LiteralExpr, "LiteralExpr"},
        {NodeType::BinaryExpr, "BinaryExpr"},
        {NodeType::Parameter, "Parameter"},
        {NodeType::Scope, "Scope"},
        {NodeType::ReturnStmt, "ReturnStmt"},
        {NodeType::FunNode, "FunNode"},
        {NodeType::VarDecl, "VarDecl"},
        {NodeType::Program, "Program"}
    };

    // a node (in it's core) just consists of it's type and the location
    // everything else is just specific sub-node metadata
    typedef struct Node {
        NodeType type;
        Span span;
        Node(NodeType t, Span s) : type(t), span(s) {};
        virtual JSON to_json() = 0;
        virtual ~Node() = default; 
    } Node;
    
    // SUB NODES BEGIN HERE

    /**
     * Leafs are used for expressions
     * they hold the value of a literal such as a number or a string
     */
    typedef struct Leaf : Node {
        std::unique_ptr<std::string> value;
        bool is_number;

        Leaf(Span s, std::unique_ptr<std::string> v, bool is_num = false) : Node(NodeType::Leaf, s), value(std::move(v)), is_number(is_num) {};

        JSON to_json() override;
    } Leaf;

    /**
     * LiteralExpr are also used for expressions
     * despire the name, they simply store the specific identifiers
     */
    typedef struct LiteralExpr : Node {
        std::unique_ptr<std::string> value;

        LiteralExpr(Span s, std::unique_ptr<std::string> value) : Node(NodeType::LiteralExpr, s), value(std::move(value)) {};

        JSON to_json() override;
    } LiteralExpr;

    /**
     * a + b
     */
    typedef struct BinaryExpr : Node {
        // left hand-side node & right hand-side
        std::unique_ptr<Node> lhs;
        std::unique_ptr<Node> rhs;
        
        // we can know what the operation is by the tokentype alone, no need for another enum class
        TokenTypes op;

        // constructor for binary expression
        BinaryExpr(
            Span s,
            std::unique_ptr<Node> lhs,
            std::unique_ptr<Node> rhs,
            TokenTypes op
        ) : Node(NodeType::BinaryExpr, s), lhs(std::move(lhs)), rhs(std::move(rhs)), op(op) {};
    
        JSON to_json() override;
    } BinaryExpr;

    // x: int
    typedef struct Parameter : Node {
        // <> (get all remaining parameters in front of it)
        bool unpack;

        // name of the variable in the parameter
        std::string_view symb;

        // type of the parameter(s)
        std::unique_ptr<Type> ptype;

        // constructor for the parameter node
        Parameter(
            Span s,
            bool u = false,
            std::string_view name = ""
        ) : Node(NodeType::Parameter, s), unpack(u), symb(name) {};
        
        JSON to_json() override;
    } Parameter;
    
    // {Scope}
    typedef struct Scope : Node {
        // the body of a scope is just a bunch of nodes, kinda like a program node
        std::vector<std::unique_ptr<Node>> body;

        // constructor for the scope node, with a default (empty) span
        explicit Scope(Span s = {}) : Node(NodeType::Scope, s) {};
        
        JSON to_json() override;
    } Scope;

    /**
     * return <expr>, <expr>, ... <end>
     */
    typedef struct ReturnStmt : Node {
        std::vector<std::unique_ptr<Node>> values;

        explicit ReturnStmt(Span s) : Node(NodeType::ReturnStmt, s) {};

        JSON to_json() override;
    } ReturnStmt;

    // function definitions require a body, parameters, a return type and a name. every single one.
    typedef struct FunNode : Node {
        // () {}
        std::vector<std::unique_ptr<Parameter>> parameters;
        std::unique_ptr<Scope> body;
        
        // void
        std::unique_ptr<Type> return_type;

        // identifier
        std::string_view name;

        // constructor for the function node (as in function definition, not declaration nor call)
        explicit FunNode(
            Span span,
            std::vector<std::unique_ptr<Parameter>> parameters, 
            std::unique_ptr<Scope> body, 
            std::unique_ptr<Type> return_type, 
            std::string_view name
        ) : Node(NodeType::FunNode, span), parameters(std::move(parameters)), body(std::move(body)), return_type(std::move(return_type)), name(name) {};
        
        JSON to_json() override;
    } FunNode;

    // variable declaration
    typedef struct VarDecl : Node {
        std::string_view name;
        bool is_const;
        bool uninitialized;
        std::unique_ptr<Type> type;
        std::unique_ptr<Node> value;

        explicit VarDecl(
            Span span,
            std::string_view name,
            bool is_const,
            bool uninitialized,
            std::unique_ptr<Type> type,
            std::unique_ptr<Node> value
        ) : Node(NodeType::VarDecl, span), name(name), is_const(is_const), uninitialized(uninitialized), type(std::move(type)), value(std::move(value)) {};

        JSON to_json() override;
    } VarDecl;
    
    // global node
    typedef struct Program : Node {
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
    } Program;

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