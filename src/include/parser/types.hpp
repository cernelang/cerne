/*
    Cerne Compiler - component of the parser, holds the types information and symbol table

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#ifndef CE_PARSER_TYPES
#define CE_PARSER_TYPES

#include "../utils.hpp"
#include "../lexer.hpp"
#include "node.hpp"

namespace cerne {
    
    // these might still be needed for future use
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

    /* 
        A path is used to break down chains of identifiers that could be types or regular executable expressions, an example might be: 
        my_space::my_member_object.specific_struct
        which is then converted into something like 
        basic_path={ {name="my_space", is_member=false, modifiers = {} }, {name="my_member_object", is_member=true, modifiers = {}}, {name="specific_struct", is_member=false, modifiers = {}} }
        "modifiers" are used for things such as calls, initializers, subscripts or generic arguments
        as order matters, you can NOT treat these as independent objects but rather one single "modifier", and since one identifier can have multiple of those
        such as: my_function<int>()[20]
        every element must have a LIST of these modifiers (with their order preserved)
    */

    enum class ModifierTypes {
        CALL,
        INITIALIZER,
        GENERIC,
        SUBSCRIPT
    };

    inline const std::map<ModifierTypes, std::string> ModifierTypesNames = {
        {ModifierTypes::CALL, "Call"},
        {ModifierTypes::INITIALIZER, "Initializer"},
        {ModifierTypes::GENERIC, "Generic"},
        {ModifierTypes::SUBSCRIPT, "Subscript"}
    };

    // define call data, initializer data, and subscript data
    struct CallData {
        std::vector<std::unique_ptr<Node>> parameters;

        JSON to_json();
    };

    struct InitializerElement {
        std::string key;
        Span key_span;

        // nodes always have spans so we don't need to worry about value_span
        std::unique_ptr<Node> value;
        
        JSON to_json();
    };

    struct InitializerData {
        bool is_keyed; // {.prop=value} or {value}
        std::vector<std::unique_ptr<InitializerElement>> values;
        
        JSON to_json();
    };

    struct SubscriptData {
        std::unique_ptr<Node> index;
        
        JSON to_json();
    };

    // forward define path
    struct Path;

    // calls, initializers and generic arguments are considered all-in-one "modifiers", since order matters and you can't treat them independently (to not lose information)
    struct Modifier {
        ModifierTypes type;
        std::variant<std::unique_ptr<CallData>, std::unique_ptr<InitializerData>, std::unique_ptr<SubscriptData>, std::vector<std::unique_ptr<Path>>> data;
    };

    struct BasicPathElement {
        std::string name;
        Span name_span;

        bool is_member;

        // you can have multiple modifiers (syntax-wise) in a single path element, however, since order is necessary to be preserved, you must treat modifiers as one single type and then have a variant for the data itself to differentiate
        std::vector<std::unique_ptr<Modifier>> modifiers;
    };

    using BasicPath = std::vector<BasicPathElement>;

    struct Path {
        BasicPath basic_path;

        // keep span information for diagnostics
        Span span;

        // if there are no calls or initializers, this path is considered a type path
        bool pure_path;
    };

    /**
     * Utility to create a very simple type
     */
    std::unique_ptr<Path> create_simple_type(const std::string& name, const Span& span = Span{0, 0, 0, 0});

    struct Symbol {
        std::string name;
        size_t scope;
        std::unique_ptr<Path> type;
    };

    // ID [u64] : Symbol
    using symbol_table = std::map<size_t, Symbol>;
}

#endif