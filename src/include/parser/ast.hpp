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
     * (span, unique_ptr<std::string> value, bool is_number)
     */
    struct Leaf : Node {
        std::unique_ptr<std::string> value;
        bool is_number;

        explicit Leaf(
            Span s, 
            std::unique_ptr<std::string> v, 
            bool is_num = false
        ) : Node(NodeType::Leaf, s), value(std::move(v)), is_number(is_num) {};

        JSON to_json() override;
    };

    /**
     * LiteralExpr are also used for expressions
     * despire the name, they simply store the specific identifiers
     * (span, unique_ptr<Path> value)
     */
    struct LiteralExpr : Node {
        std::unique_ptr<Path> value;

        explicit LiteralExpr(
            Span s, 
            std::unique_ptr<Path> value
        ) : Node(NodeType::LiteralExpr, s), value(std::move(value)) {};

        JSON to_json() override;
    };

    /**
     * PrefixExpr is used for unary operators
     * (span, unique_ptr<Node> value, TokenTypes op)
     */
    struct PrefixExpr : Node {
        std::unique_ptr<Node> value;
        TokenTypes op;

        explicit PrefixExpr(
            Span s, 
            std::unique_ptr<Node> value, 
            TokenTypes op
        ) : Node(NodeType::PrefixExpr, s), value(std::move(value)), op(op) {};

        JSON to_json() override;
    };

    /**
     * SuffixExpr is used for postfix operators
     * (span, unique_ptr<Node> value, TokenTypes op)
     */
    struct SuffixExpr : Node {
        std::unique_ptr<Node> value;
        TokenTypes op;

        explicit SuffixExpr(
            Span s, 
            std::unique_ptr<Node> value, 
            TokenTypes op
        ) : Node(NodeType::SuffixExpr, s), value(std::move(value)), op(op) {};

        JSON to_json() override;
    };

    /**
     * a <operator> b
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

        // for the other types of BinaryExpr
        protected:
            explicit BinaryExpr(
                NodeType type,
                Span s,
                std::unique_ptr<Node> lhs,
                std::unique_ptr<Node> rhs,
                TokenTypes op
            ) : Node(type, s), lhs(std::move(lhs)), rhs(std::move(rhs)), op(op) {};
    };

    /**
     * a = b
     * a += b
     * ...
     */
    struct AssignmentExpr : BinaryExpr {
        explicit AssignmentExpr(
            Span s,
            std::unique_ptr<Node> lhs,
            std::unique_ptr<Node> rhs,
            TokenTypes op
        ) : BinaryExpr(NodeType::AssignmentExpr, s, std::move(lhs), std::move(rhs), op) {};
        
        // to_json is already implemented in BinaryExpr, no need to override it here
    };

    /**
     * a < b
     * a > b
     * ...
     */
    struct ComparisonExpr : BinaryExpr {
        explicit ComparisonExpr(
            Span s,
            std::unique_ptr<Node> lhs,
            std::unique_ptr<Node> rhs,
            TokenTypes op
        ) : BinaryExpr(NodeType::ComparisonExpr, s, std::move(lhs), std::move(rhs), op) {};
        
        // to_json is already implemented in BinaryExpr, no need to override it here
    };

    /**
     * a .. b
     */
    struct RangeExpr : BinaryExpr {
        explicit RangeExpr(
            Span s,
            std::unique_ptr<Node> lhs,
            std::unique_ptr<Node> rhs
        ) : BinaryExpr(NodeType::RangeExpr, s, std::move(lhs), std::move(rhs), TokenTypes::RANGE) {};
        
        // to_json is already implemented in BinaryExpr, no need to override it here
    };

    // x: int
    struct Parameter : Node {
        // ... (get all remaining parameters in front of it)
        bool unpack;

        // name of the variable in the parameter
        std::string name;

        // name span for diagnostics
        Span name_span;

        // type of the parameter(s)
        std::unique_ptr<Path> ptype;

        // constructor for the parameter node
        explicit Parameter(
            Span s,
            bool u = false,
            const std::string& name = "",
            const Span& name_span = {}
        ) : Node(NodeType::Parameter, s), unpack(u), name(name), name_span(name_span) {};
        
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

        // again for diagnostics
        Span name_span;

        // constructor for the function node (as in function definition, not declaration nor call)
        explicit FunNode(
            const Span& span,
            std::vector<std::unique_ptr<Parameter>> parameters, 
            std::unique_ptr<Scope> body, 
            std::unique_ptr<Path> return_type, 
            const std::string& name = "",
            const Span& name_span = {}
        ) : Node(NodeType::FunNode, span), parameters(std::move(parameters)), body(std::move(body)), return_type(std::move(return_type)), name(name), name_span(name_span) {};
        
        JSON to_json() override;
    };

    // variable declaration
    struct VarDecl : Node {
        std::string name;
        Span name_span;
        bool is_const;
        bool uninitialized;
        std::unique_ptr<Path> var_type;
        std::unique_ptr<Node> value;

        explicit VarDecl(
            const Span& span,
            const std::string& name = "",
            const Span& name_span = {},
            bool is_const = false,
            bool uninitialized = false,
            std::unique_ptr<Path> var_type = nullptr,
            std::unique_ptr<Node> value = nullptr
        ) : Node(NodeType::VarDecl, span), name(name), name_span(name_span), is_const(is_const), uninitialized(uninitialized), var_type(std::move(var_type)), value(std::move(value)) {};

        JSON to_json() override;
    };

    // condition block
    struct ConditionBlock : Node {
        std::vector<std::pair<std::unique_ptr<Node>, std::unique_ptr<Scope>>> conditions;
        std::unique_ptr<Scope> else_block;

        explicit ConditionBlock(const Span& span) : Node(NodeType::ConditionBlock, span), conditions(), else_block(nullptr) {};

        JSON to_json() override;
    };

    // while node
    struct WhileNode : Node {
        std::unique_ptr<Node> condition;
        std::unique_ptr<Scope> body;

        explicit WhileNode(
            const Span& span,
            std::unique_ptr<Node> condition,
            std::unique_ptr<Scope> body
        ) : Node(NodeType::WhileNode, span), condition(std::move(condition)), body(std::move(body)) {};

        JSON to_json() override;
    };

    // for node
    struct ForNode : Node {
        // c style loop
        std::unique_ptr<Node> init;
        std::unique_ptr<Node> condition;
        std::unique_ptr<Node> update;

        // range style loop
        std::unique_ptr<Node> variable;
        std::unique_ptr<Node> range;
        
        std::unique_ptr<Scope> body;
        
        bool is_range_loop;
        explicit ForNode(
            const Span& span,
            std::unique_ptr<Node> init = nullptr,
            std::unique_ptr<Node> condition = nullptr,
            std::unique_ptr<Node> update = nullptr,
            std::unique_ptr<Node> variable = nullptr,
            std::unique_ptr<Node> range = nullptr,
            std::unique_ptr<Scope> body = nullptr,
            bool is_range_loop = false
        ) : Node(NodeType::ForNode, span), 
            init(std::move(init)), 
            condition(std::move(condition)), 
            update(std::move(update)), 
            variable(std::move(variable)), 
            range(std::move(range)), 
            body(std::move(body)),
            is_range_loop(is_range_loop) {};

        JSON to_json() override;
    };

    // import and export nodes

    struct ImportNode : Node {
        std::string file_path;
        Span file_path_span{0,0,0,0};
        std::string user;
        Span user_span{0,0,0,0};
        std::vector<std::string> package_path{};
        std::vector<Span> package_path_spans{};
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