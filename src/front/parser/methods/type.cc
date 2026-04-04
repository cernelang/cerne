/*
    Cerne Compiler - component of the parser.

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#include "../../../include/parser.hpp"

// parse_type utils

/**
 * since dot and member_access have the same logic and only differ one parameter (is_member)
 * we can combine the logic into a single function instead of copy pasting for both cases
 * makes the code much cleaner and easier to maintain
 */
void push_type_path_element(cerne::ParseMachine* machine, cerne::Type* type, bool is_member, cerne::TokenTypes token_type) {
    // if we encounter a member access token, it means that we're still parsing the same complex type, so we just need to add the next identifier to the current type path
    machine->offset++;

    // check for EOF and whether the next token is an identifier (it has to be an identifier since member access can only be followed by another identifier, otherwise it's an error)
    if(machine->expect(cerne::TokenTypes::IDENTIFIER)) return;

    const auto& next = machine->list[machine->offset];
    if(next.type != cerne::TokenTypes::IDENTIFIER && machine->options.flags.find("quiet") == machine->options.flags.end()) {
        cerne::cerror(
            machine->file_path,
            ERR_UNEXPECTED_TOKEN,
            std::format("Expected Identifier, instead got {} at {}:{}", cerne::TokenTypeNames.at(next.type), next.span.line, next.span.col),
            cerne::code_snippet(
                machine->code_sv,
                next.span,
                std::format("`{}` is not a valid type path element.", cerne::TokenTypeNames.at(next.type))
            ),
            next.span
        );

        machine->errors++;
        return;
    }

    // if it is an identifier, we just add it to the current type path as a member (since it's after a member access token)
    std::string type_name = *next.value.get();

    // before pushing tho, we check if the current typeinfo is actually a typepath since primitives cannot contain members (generates an error)
    if(type->data != cerne::TypeData::COMPLEX || !std::holds_alternative<cerne::TypePath>(type->typeinfo)) {
        cerne::cerror(
            machine->file_path,
            ERR_UNEXPECTED_SYMBOL,
            std::format("Unexpected member access token at {}:{}. Member access tokens can only be used with complex types, but the current type is {}.", next.span.line, next.span.col, cerne::TokenTypeNames.at(token_type)),
            cerne::code_snippet(machine->code_sv, next.span, std::format("The current type is not a complex type.")),
            next.span
        );

        machine->errors++;
        return;
    }

    // if it is a type path, we just push the new identifier as a member to the current type path
    std::get<cerne::TypePath>(type->typeinfo).push_back(cerne::TypePathElement{
        .name=type_name,
        .is_member=is_member
    });

    return;
}

/**
 * subparse function to parse types ALONE.
 * x: <type>
 * types can be 3 main things:
 * - primitive (int, i32, i8, u8, ...)
 * - complex (a "custom" struct for example)
 * - compound (templated types such as List<int> or Range<int, int>)
 * is_nested is important for the recursion mechanism to work properly
 */
std::unique_ptr<cerne::Type> cerne::ParseMachine::parse_type(bool is_nested = false) {
    // first token MUST BE an identifier
    const auto& first = list[offset];

    if(first.type != TokenTypes::IDENTIFIER && options.flags.find("quiet") == options.flags.end()) {
        cerne::cerror(
            file_path,
            ERR_UNEXPECTED_TOKEN,
            std::format("Expected Identifier, instead got {} at {}:{}", cerne::TokenTypeNames.at(first.type), first.span.line, first.span.col),
            cerne::code_snippet(code_sv, first.span, std::format("Expected an identifier, got {}.", cerne::TokenTypeNames.at(first.type))),
            first.span
        );

        errors++;
        return nullptr;
    }

    // now we start our main type loop where we check for the identifier, check whether it's a primitive type or start of a complex type
    // and then check for template types if any
    auto type = std::make_unique<cerne::Type>();
    while(offset < list.size()) {
        const auto& token = list[offset];
        const auto& token_type = token.type;
        bool should_break = false;

        switch(token_type) {
            
            // identifier (regular "word" type)
            case TokenTypes::IDENTIFIER: {
                std::string type_name = *token.value.get();
                
                // check if it's a primitive type
                if(cerne::primitive_types.find(type_name) != primitive_types.end()) {
                    // if it is a primitive type, create the primitive type struct and update our type's parameters (we won't leave the loop since there are templated primitive types) 
                    auto primitive = primitive_types.at(type_name);
                    type->data = cerne::TypeData::PRIMITIVE;
                    type->typeinfo = primitive;
                    break;
                }

                // if it's not, then it has to be a complex type (for now, since compound types are just complex types with templated types, we don't need to differentiate yet)
                type->data = cerne::TypeData::COMPLEX;
                type->typeinfo = TypePath{
                    TypePathElement{
                        .name=type_name,
                        .is_member=false
                    }
                };

                break;
            }

            // :: (member access)
            case TokenTypes::MEMBER_ACCESS: {
                push_type_path_element(this, type.get(), true, token_type);
                break;
            }

            // . (regular dot, it's a property, not member access, so it should add to the type path but with the is_member parameter as false)
            case TokenTypes::DOT: {
                push_type_path_element(this, type.get(), false, token_type);
                break;
            }

            // < (start of template type parameters)
            case TokenTypes::LESS_THAN: {
                // in case of a template type, it doesn't matter whether it's a primitive or complex type
                // since both can be templated (for example, my_template<int> or List<int>)
                offset++;

                if(!expect(TokenTypes::IDENTIFIER)) return nullptr;

                // now, we do need a recursion solution since template types can be nested (for example, my_template<List<int>>)
                // so we just call parse_type() again to get the templated type and then update
                auto templated_type = parse_type(true);
                if(templated_type == nullptr) return nullptr;
                type->templated_type = std::move(templated_type);

                // we change type's data to compound since it is now a template (primitive) type and not a "raw" primitive type anymore
                if(type->data == TypeData::PRIMITIVE) type->data = TypeData::COMPOUND;
                break;
            }

            // > (end of a template type)
            case TokenTypes::GREATER_THAN: {
                // if it's nested, we just return the type (thanks to recursion)
                if(is_nested) {
                    offset++;
                    return type;
                } else {
                    // syntax error
                    cerne::cerror(
                        file_path,
                        ERR_UNEXPECTED_TOKEN,
                        std::format("Unexpected token {} at {}:{}. The '>' token is only expected in nested template types.", cerne::TokenTypeNames.at(token_type), token.span.line, token.span.col),
                        cerne::code_snippet(code_sv, token.span, std::format("Unexpected token {}.", cerne::TokenTypeNames.at(token_type))),
                        token.span
                    );

                    errors++;
                    break;
                }
            }

            // * (for pointers)
            case TokenTypes::MUL: {
                type->is_pointer = true;
                break;
            }

            // if there isn't a valid token for a type declaration, we just break out of the loop
            default: {
                should_break = true;
            }
        }

        if(should_break) break;

        offset++;
    }

    return type;
}