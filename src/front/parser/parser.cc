/*
    Cerne Compiler - Parser Component, responsible for converting the token list from the previous lexer component to a more comprehensible node tree (AST).

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#include "../include/parser.hpp"
#include "../include/parser/handler.hpp"

const std::map<std::string, std::function<std::unique_ptr<cerne::Node>(const cerne::blueprint_arguments&)>> blueprints = {
    { "return", static_cast<std::unique_ptr<cerne::Node>(*)(const cerne::blueprint_arguments&)>(cerne::Return) },
    { "fun", static_cast<std::unique_ptr<cerne::Node>(*)(const cerne::blueprint_arguments&)>(cerne::Fun) },
    { "let", static_cast<std::unique_ptr<cerne::Node>(*)(const cerne::blueprint_arguments&)>(cerne::Let) },
    { "const", static_cast<std::unique_ptr<cerne::Node>(*)(const cerne::blueprint_arguments&)>(cerne::_Const) }
};

/* --- Parse Machine Methods Begin --- */

bool cerne::ParseMachine::expect(TokenTypes type, bool just_check) {
    if(is_eof() && options.flags.find("quiet") == options.flags.end()) {
        cerne::cerror(
            file_path,
            ERR_UNEXPECTED_EOF,
            std::format("Expected {}, but reached end of file.", TokenTypeNames.at(type)),
            "EOF",
            Span{
                .line = 0,
                .col = 0,
                .offset = code_sv.size(),
                .length = 0
            }
        );
        errors++;
        return false;
    }

    const auto& token = peek();
    if(token.type != type) {
        if(!just_check) {
            if(options.flags.find("quiet") == options.flags.end()) {
                cerne::cerror(
                    file_path,
                    ERR_UNEXPECTED_TOKEN,
                    std::format("Expected {}, instead got {} at {}:{}", TokenTypeNames.at(type), TokenTypeNames.at(token.type), token.span.line, token.span.col),
                    cerne::code_snippet(
                        code_sv,
                        token.span,
                        std::format("`{}` is not a {}.", token.value ? *(token.value) : TokenTypeNames.at(token.type), TokenTypeNames.at(type))
                    ),
                    token.span
                );
            }

            skip_to_next_end();
            errors++;
        }

        return false;
    }
    return true;
}

bool cerne::ParseMachine::expect_or(std::vector<TokenTypes> types) {
    for(const auto& type : types) {
        if(expect(type, true)) {
            return true;
        }
    }

    if(options.flags.find("quiet") == options.flags.end()) {
        const auto& token = peek();
        std::string expected_types_str;
        for(size_t i = 0; i < types.size(); i++) {
            expected_types_str += TokenTypeNames.at(types[i]);
            if(i < types.size() - 1) {
                expected_types_str += "` or `";
            }
        }
        
        cerne::cerror(
            file_path,
            ERR_UNEXPECTED_TOKEN,
            std::format("Expected one of `{}`, instead got {} at {}:{}", expected_types_str, TokenTypeNames.at(token.type), token.span.line, token.span.col),
            cerne::code_snippet(
                code_sv,
                token.span,
                std::format("`{}` is not one of `{}`.", token.value ? *(token.value) : TokenTypeNames.at(token.type), expected_types_str)
            ),
            token.span
        );

        skip_to_next_end();
        errors++;
    }
    
    return false;
}


/**
 * Execute mnemonic blueprint
 */
std::unique_ptr<cerne::Node>  cerne::ParseMachine::parse_mnemonic() {
    const auto& token = list[offset];
    std::string token_name = std::string(token.value.get()->c_str());
    if(blueprints.find(token_name) != blueprints.end()) {
        auto arguments = cerne::blueprint_arguments{
            .machine=this
        };

        return blueprints.at(token_name)(arguments);
    } else {
        cerne::cerror(
            file_path, 
            ERR_UNKNOWN_KEYWORD,
            std::format("Unknown keyword `{}` at {}:{}", token_name, token.span.line, token.span.col), 
            cerne::code_snippet(code_sv, token.span, std::format("`{}` does not exist yet.", *(token.value))),    
            token.span
        );
        errors++;
    }
    return nullptr;
}

/**
 * Subparse method responsible for parsing a scope
 */
std::unique_ptr<cerne::Scope> cerne::ParseMachine::parse_scope() {
    auto& start_token = list[offset];
    offset++;

    auto scope = std::make_unique<cerne::Scope>();

    // parse everything until end of scope
    while(list[offset].type != TokenTypes::END_SCOPE && offset < list.size()) {
        auto node = parse(list[offset]);
        if(node) scope->body.push_back(std::move(node));
        offset++;
    }
    
    // if file ended without closing scope
    if(list[offset].type != TokenTypes::END_SCOPE && offset >= list.size() && options.flags.find("quiet") == options.flags.end()) {
        cerne::cerror(
            file_path,
            ERR_OPEN_SCOPE,
            std::format("Syntax Error: Unclosed scope. Expected {}'}}'{} to match the opening brace.", BG "219m" FG "233m", RESET ESC "[1;37m"),
            cerne::code_snippet(
                code_sv,
                start_token.span,
                "Scope begins here without closure (no matching '}' for this '{')"
            ),
            start_token.span
        );
        errors++;
    }

    return scope;
}

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

// --- parse_parameter() Utils ---

bool checkIdentifier(cerne::ParseMachine* machine, const cerne::Token& id) {
    if(id.type != cerne::TokenTypes::IDENTIFIER  && machine->options.flags.find("quiet") == machine->options.flags.end()) {
        cerne::cerror(
            machine->file_path, 
            ERR_UNEXPECTED_TOKEN,
            std::format("Expected {} or {}, instead got {} at {}:{}", 
                cerne::TokenTypeNames.at(cerne::TokenTypes::UNPACK), 
                cerne::TokenTypeNames.at(cerne::TokenTypes::IDENTIFIER), 
                cerne::TokenTypeNames.at(id.type), 
                id.span.line, 
                id.span.col
            ), 
            cerne::code_snippet(
                machine->code_sv,
                id.span,
                std::format("`{}` is not a valid parameter name.", cerne::TokenTypeNames.at(id.type))
            ),
            id.span
        );
        machine->errors++;
        return false;
    }

    return true;
}

std::unique_ptr<cerne::Parameter> define_parameter(bool isunpack, const cerne::Token& token) {
    // initialize a parameter
    auto param = std::make_unique<cerne::Parameter>(
        token.span,
        isunpack,
        *token.value.get()
    );

    // change the parameter type to "any" by default
    param->ptype = std::make_unique<cerne::Type>(cerne::Type{
        .data = cerne::TypeData::PRIMITIVE,
        .typeinfo = cerne::Primitive::Any
    });

    return param;
}

/**
 * Helper util to update the parameter node with type information in case there is any
 */
void update_parameter(cerne::ParseMachine* machine, cerne::Parameter* param) {
    const auto& define = machine->list[machine->offset];

    // if there isn't a type in front of the parameter identifier (like param_name instead of param_name: int), just return
    if(define.type != cerne::TokenTypes::DEFINE) return;

    // if there is, we move on to the next token, check for EOF and then parse the type and update the parameter's ptype
    machine->offset++;

    if(!machine->expect(cerne::TokenTypes::IDENTIFIER)) return;

    auto type = machine->parse_type();
    if(type->data != cerne::TypeData::UNKNOWN) {
        param->ptype=std::move(type);
    }
}

/**
 * Parameter syntax:
 * <>ParamName: int         (all following parameters are of type integer)
 * MyParam: int             (MyParam is of type int)
 * Parameter                (when no type is explicitly declared, compiler assumes "any")
 */
std::unique_ptr<cerne::Parameter> cerne::ParseMachine::parse_parameter() {
    // first token MUST BE an unpack token or an identifier.
    if(!expect(TokenTypes::UNPACK) && !expect(TokenTypes::IDENTIFIER)) return nullptr;

    const auto& first = list[offset];

    if(first.type == TokenTypes::UNPACK) {
        offset++;
        if(!expect(TokenTypes::IDENTIFIER)) return nullptr;

        const auto& id = list[offset];
        // get identifier (if the next token is one) and define the parameter and push it
        if(!checkIdentifier(this, id)) return nullptr;
        auto param = define_parameter(true, id);
        update_parameter(this, param.get());
        return param;
    }

    if(!checkIdentifier(this, first)) return nullptr;

    auto param = define_parameter(false, first);
    update_parameter(this, param.get());
    return param;
}

/**
 * Global parse function, executes appropriate function depending on the current token
 */
std::unique_ptr<cerne::Node> cerne::ParseMachine::parse(cerne::Token& token) {
    switch(token.type) {
        case cerne::TokenTypes::MNEMONIC: 
            return parse_mnemonic();

        case cerne::TokenTypes::START_SCOPE:
            return parse_scope();

        default:
            // how?? 
            break;
    }

    return nullptr;
}

void cerne::ParseMachine::walk() {
    for(; offset < list.size(); offset++) {
        auto& token = list[offset];
        auto node = parse(token);
        if(node) ast->root->node_list.push_back(std::move(node));
    }
}

/* --- Parse Machine Methods Over --- */

/* --- main parse function --- */

std::unique_ptr<cerne::AST> cerne::parse(const std::string_view& code_sv, cerne::tokenlist& list, const char* path, const cerne::args& options) {
    // initialize the AST and the machine
    auto ast = std::make_unique<AST>(path);
    auto machine = std::make_unique<ParseMachine>(ast.get(), list, path, options, code_sv);

    // begin machine walk
    machine->walk();

    // before returning, pass onto the AST the errors/warnings (important for the CLI's main build loop)
    ast->errors     =   machine->errors;
    ast->warnings   =   machine->warnings;

    return ast;
}