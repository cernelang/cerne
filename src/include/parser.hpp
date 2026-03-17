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
    enum class Primitive {
        // Defaults
        Void, Auto, Any,

        // Literals
        I8, I16, I32, I64,
        U8, U16, U32, U64,
        Char, Str, List, Map,
        
        // Concurrency
        Signal,

        // Misc
        Object, Function, Space
    };

    const std::map<std::string, Primitive> primitive_types = {
        {"void", Primitive::Void},
        {"auto", Primitive::Auto},
        {"any", Primitive::Any},

        {"i8", Primitive::I8},
        {"i16", Primitive::I16},
        {"i32", Primitive::I32},
        {"i64", Primitive::I64},
        {"u8", Primitive::U8},
        {"u16", Primitive::U16},
        {"u32", Primitive::U32},
        {"u64", Primitive::U64},
        {"char", Primitive::Char},
        {"str", Primitive::Str},
        {"List", Primitive::List},
        {"Map", Primitive::Map},

        {"Signal", Primitive::Signal},
        {"Object", Primitive::Object},
        {"Function", Primitive::Function},
        {"Space", Primitive::Space}
    };

    const std::map<std::string, Primitive> primitive_compound = {
        {"List", Primitive::List},
        {"Map", Primitive::Map},
        {"Signal", Primitive::Signal}
    };
    
    enum class TypeData {
        PRIMITIVE,
        COMPOUND,
        COMPLEX,
        UNKNOWN
    };

    const std::map<TypeData, std::string> TypeDataNames = {
        {TypeData::PRIMITIVE, "Primitive"},
        {TypeData::COMPOUND, "Compound"},
        {TypeData::COMPLEX, "Complex"},
        {TypeData::UNKNOWN, "Unknown"},
    };

    /* 
        A type path is used to break down complex types such as 
        my_space::my_member_object.specific_struct
        and converts into something like 
        {"my_space" (is_member=false),"my_member_object" (is_member=true),"specific_struct" (is_member=false)}
    */

    typedef struct TypePathElement {
        std::string_view name;
        // not member? it's probably a property access then
        bool is_member;
    } TypePathElement;
    
    using TypePath = std::vector<TypePathElement>;

    typedef struct Type {
        TypeData data;

        // by default, a type is NOT const
        bool is_const = false;
        bool is_pointer = false;
        
        // variants are better than unions
        std::variant<Primitive, TypePath> typeinfo;
        
        // by default, types are also not templates (obviously)
        // they can tho, which is why it's an "optional" field
        std::unique_ptr<Type> templated_type = nullptr;
    } Type;

    typedef struct Symbol {
        std::string_view name;
        size_t scope;
        Type type;
    } Symbol;

    // now we start declaring what a node even is
    enum class NodeType {
        Leaf,
        LiteralExpr,
        BinaryExpr,
        Parameter,
        Scope,
        ReturnStmt,
        FunNode,
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
        {NodeType::Program, "Program"}
    };

    // a node (in it's core) just consists of it's type and the location
    // everything else is just specific sub-node metadata
    typedef struct Node {
        NodeType type;
        Span span;
        Node(NodeType t, Span s) : type(t), span(s) {};
        virtual std::string print(size_t identation) = 0;
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

        std::string print(size_t identation) override;
    } Leaf;

    /**
     * LiteralExpr are also used for expressions
     * despire the name, they simply store the specific identifiers
     */
    typedef struct LiteralExpr : Node {
        std::unique_ptr<std::string> value;

        LiteralExpr(Span s, std::unique_ptr<std::string> value) : Node(NodeType::LiteralExpr, s), value(std::move(value)) {};

        std::string print(size_t identation) override;
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
    
        std::string print(size_t identation) override;
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
        
        std::string print(size_t identation) override;
    } Parameter;
    
    // {Scope}
    typedef struct Scope : Node {
        // the body of a scope is just a bunch of nodes, kinda like a program node
        std::vector<std::unique_ptr<Node>> body;

        // constructor for the scope node, with a default (empty) span
        Scope(Span s = {}) : Node(NodeType::Scope, s) {};
        
        std::string print(size_t identation) override;
    } Scope;

    /**
     * return <expr>, <expr>, ... <end>
     */
    typedef struct ReturnStmt : Node {
        std::vector<std::unique_ptr<Node>> values;

        ReturnStmt(Span s) : Node(NodeType::ReturnStmt, s) {};

        std::string print(size_t identation) override {
            std::string result = std::string(identation, ' ') + "ReturnStmt:\n";
            for(const auto& value : values) {
                result += value->print(identation + 2) + "\n";
            }
            return result;
        };
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
        FunNode(
            Span span,
            std::vector<std::unique_ptr<Parameter>> parameters, 
            std::unique_ptr<Scope> body, 
            std::unique_ptr<Type> return_type, 
            std::string_view name
        ) : Node(NodeType::FunNode, span), parameters(std::move(parameters)), body(std::move(body)), return_type(std::move(return_type)), name(name) {};
        
        std::string print(size_t identation) override;
    } FunNode;
    
    // global node
    typedef struct Program : Node {
        std::vector<std::unique_ptr<Node>> node_list;

        // constructor for the program node, no need for a span since it's just a container for the whole program
        Program() : Node(NodeType::Program, Span{0,0,0,0}) {};
        
        std::string print(size_t identation) override {return std::string(identation, ' ');};
    } Program;
    
    // ID [u64] : Symbol
    typedef std::map<size_t, Symbol> symbol_table;

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
            
            // stores the actual
            std::unique_ptr<Program> root;
            
            // for Diag
            const char* file_path;

            AST(const char* path): file_path(path) {
                root = std::make_unique<Program>();
                errors = 0;
                warnings = 0;
            };
            ~AST()=default;
    };

    /**
     * The parse machine class is responsible for 
     * containing the state of the parser
     * containing all the subparse methods
     * and also for walking through the token list and executing the appropriate subparse method depending on the current token
     * (it also contains the error/warning count for the AST)
     * (and it also contains the file path and the code string view for diagnostics)
     */
    class ParseMachine {
        public:
            size_t offset = 0;
            size_t errors = 0;
            size_t warnings = 0;
            size_t scope = 0;
            cerne::AST* ast;
            cerne::tokenlist& list;
            const char* file_path;
            const cerne::args& options;
            const std::string_view& code_sv;

            // constructor & destructor
            ParseMachine(
                cerne::AST* _ast, 
                cerne::tokenlist& _list, 
                const char* _file_path, 
                const cerne::args& _options,
                const std::string_view& _code_sv
            )
                : ast(_ast), list(_list), file_path(_file_path), options(_options), code_sv(_code_sv) {};
            ~ParseMachine()=default;

            // checks
            bool check_eof(const std::string_view& expected);

            // subparse methods
            void parse_mnemonic();
            std::unique_ptr<cerne::Scope> parse_scope();
            std::unique_ptr<Parameter> parse_parameter();
            std::unique_ptr<Type> parse_type(bool is_nested);
            std::unique_ptr<cerne::Node> parse_nud();
            std::unique_ptr<cerne::Node> parse_infix(std::unique_ptr<cerne::Node> lhs);
            std::unique_ptr<cerne::Node> parse_expr(size_t precedence);
            void parse(cerne::Token& token);
            void walk();
    };

    [[nodiscard]] constexpr size_t get_score(TokenTypes type) noexcept;
    std::unique_ptr<AST> parse(const std::string_view& code_sv, tokenlist& list, const char* file_path, const args& options);
}

#endif