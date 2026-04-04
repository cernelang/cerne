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
        std::string_view name;
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

    struct Symbol {
        std::string_view name;
        size_t scope;
        Type type;
    };

    // ID [u64] : Symbol
    using symbol_table = std::map<size_t, Symbol>;
}

#endif