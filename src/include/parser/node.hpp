/*
    Cerne Compiler - component of the parser, holds the definition of a node

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#ifndef CERNE_NODE
#define CERNE_NODE

#include "../utils.hpp"
#include "../lexer.hpp"

namespace cerne {
    // we begin by declaring what a node even is (node.hpp)
    enum class NodeType {
        Leaf,
        LiteralExpr,
        BinaryExpr,
        AssignmentExpr,
        ComparisonExpr,
        PrefixExpr,
        SuffixExpr,
        Parameter,
        Scope,
        ReturnStmt,
        FunNode,
        VarDecl,
        ConditionBlock,
        Import,
        Export,
        Program
    };

    // nodetypenames
    const std::map<NodeType, std::string> NodeTypeNames = {
        {NodeType::Leaf, "Leaf"},
        {NodeType::LiteralExpr, "LiteralExpr"},
        {NodeType::BinaryExpr, "BinaryExpr"},
        {NodeType::AssignmentExpr, "AssignmentExpr"},
        {NodeType::ComparisonExpr, "ComparisonExpr"},
        {NodeType::PrefixExpr, "PrefixExpr"},
        {NodeType::SuffixExpr, "SuffixExpr"},
        {NodeType::Parameter, "Parameter"},
        {NodeType::Scope, "Scope"},
        {NodeType::ReturnStmt, "ReturnStmt"},
        {NodeType::FunNode, "FunNode"},
        {NodeType::VarDecl, "VarDecl"},
        {NodeType::ConditionBlock, "ConditionBlock"},
        {NodeType::Import, "Import"},
        {NodeType::Export, "Export"},
        {NodeType::Program, "Program"}
    };

    // a node (in it's core) just consists of it's type and the location
    // everything else is just specific sub-node metadata
    struct Node {
        NodeType type;
        Span span;
        Node(NodeType t, Span s) : type(t), span(s) {};
        virtual JSON to_json() = 0;
        virtual ~Node() = default; 
    };
}

#endif