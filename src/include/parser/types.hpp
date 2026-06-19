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

    struct TypePathElement {
        std::string name;
        // not member? it's probably a property access then
        bool is_member;
    };
    
    using TypePath = std::vector<TypePathElement>;

    struct Type {
        TypeData data;

        // by default, a type is NOT const
        bool is_const = false;
        bool is_pointer = false;
        
        // variants are better than unions
        std::variant<Primitive, TypePath> typeinfo;
        
        // by default, types are also not templates (obviously)
        // they can tho, which is why it's an "optional" field
        std::unique_ptr<Type> templated_type = nullptr;
    };

    struct CallData {
        std::vector<std::unique_ptr<Node>> parameters;
    };

    struct InitializerElement {
        std::string key;
        Span key_span;

        // nodes always have spans so we don't need to worry about value_span
        std::unique_ptr<Node> value;
    };

    struct InitializerData {
        bool is_keyed; // {.prop=value} or {value}
        std::vector<InitializerElement> values;
    };

    // forward define path
    struct Path;

    struct BasicPathElement {
        std::string name;
        bool is_member;

        // calls/class initializers can be part of the path
        std::unique_ptr<CallData> call = nullptr;
        std::unique_ptr<InitializerData> initializer = nullptr;
        
        // generics can also be part of the path, so for example my_function<int, str>()
        std::vector<std::unique_ptr<Path>> generic_args = {};
    };

    using BasicPath = std::vector<BasicPathElement>;

    struct Path {
        BasicPath basic_path;

        // if there are no calls or initializers, this path is considered a type path
        bool pure_path;
    };

    struct Symbol {
        std::string name;
        size_t scope;
        Type type;
    };

    // ID [u64] : Symbol
    using symbol_table = std::map<size_t, Symbol>;
}

#endif