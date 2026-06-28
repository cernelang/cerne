/*
    Cerne Compiler - Component of the parser. Responsible for parsing path elements in types, expressions and calls.

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#include "../../../include/parser.hpp"

using enum cerne::TokenTypes;

/**
 * Elements must have an identifier as their name, and then optionally a call, initializer, generic arguments, or a mix of all of them.
 * The order is important as identifiers MUST come first and then modifiers (calls, initializers, generics) can come in the order necessary.
 */
std::unique_ptr<cerne::BasicPathElement> parse_path_element(cerne::ParseMachine* machine, bool* pure) {
    auto element = std::make_unique<cerne::BasicPathElement>();

    auto& token = machine->peek();
    if(token.type != IDENTIFIER) {
        cerne::cerror(
            machine->file_path, 
            ERR_UNEXPECTED_SYMBOL,
            std::format("Unexpected token `{}` at {}:{}", *(token.value), token.span.line, token.span.col), 
            cerne::code_snippet(machine->code_sv, token.span, std::format("`{}` is not a valid path element.", *(token.value))),    
            token.span
        );
        machine->errors++;
        return nullptr;
    }

    element->name = *(token.value);

    // now loop through modifiers
    while(machine->offset < machine->list.size()) {
        auto& token = machine->peek();

        // (nodes, nodes, ...) (calls)
        if(token.type == START_PARAM) {
            // immediately set pure to false since we have a call
            *pure = false;

            // create call data
            auto call_data = std::make_unique<cerne::CallData>();
            
            // now, we parse the nodes by simply calling parse() until we encounted a ) (and for every , we call it again)
            while(machine->offset < machine->list.size()) {
                auto node = machine->parse(machine->peek());
                if(!node) {
                    // break since we can't parse the current argument
                    break;
                }

                // add node to call data
                call_data->parameters.push_back(std::move(node));

                // continue IF we have a , otherwise we break
                if(machine->peek().type == COMMA) {
                    machine->advance();
                    continue;
                } 

                break;
            }

            // check if we have a ) to close the call
            if(machine->peek().type != END_PARAM) {
                cerne::cerror(
                    machine->file_path,
                    ERR_UNEXPECTED_SYMBOL,
                    std::format("Unexpected token `{}` at {}:{}", *(token.value), token.span.line, token.span.col),
                    cerne::code_snippet(machine->code_sv, token.span, std::format("`{}` is not a valid path element.", *(token.value))),
                    token.span
                );
                machine->errors++;
                return nullptr;
            } else {
                // advance past the )
                machine->advance();

                // create a modifier based on the call data and add it to the element
                auto modifier = std::make_unique<cerne::Modifier>();
                modifier->type = cerne::ModifierTypes::CALL;
                modifier->data = std::move(call_data);
                element->modifiers.push_back(std::move(modifier));
            }

            // continue to the next possible modifier
            continue;
        }

        // {.something=true} or {true} (initializers)
        if(token.type == START_SCOPE) {
            // in this case we also set pure to false since we have an initializer
            *pure = false;

            // create initializer data
            auto initializer_data = std::make_unique<cerne::InitializerData>();

            // default to be false
            initializer_data->is_keyed = false;

            // now, when parsing the elements, initializers are different since we FIRST need to check if they're keyed, then we loop based on that in a fashion of either:
            // DOT IDENTIFIER EQUALS NODE COMMA or NODE COMMA (or the same but without the COMMA if it's the last element)
            while(machine->offset < machine->list.size()) {
                auto& token = machine->peek();

                // keyed
                if(token.type == DOT) {
                    // if our initializer is unkeyed, makes no sense to have a keyed element, so we throw an error
                    if(initializer_data->values.size() > 0 && !initializer_data->is_keyed) {
                        cerne::cerror(
                            machine->file_path,
                            ERR_UNEXPECTED_SYMBOL,
                            std::format("Unexpected token `{}` at {}:{}", *(token.value), token.span.line, token.span.col),
                            cerne::code_snippet(machine->code_sv, token.span, std::format("`{}` is not a valid path element.", *(token.value))),
                            token.span
                        );
                        machine->errors++;
                        return nullptr;
                    }

                    // if such is not the case, we simply set the initializer data to be keyed and then we continue to parse the next token as a key
                    initializer_data->is_keyed = true;
                    machine->advance();

                    auto& key = machine->peek();
                    if(key.type != IDENTIFIER) {
                        cerne::cerror(
                            machine->file_path,
                            ERR_UNEXPECTED_SYMBOL,
                            std::format("Unexpected token `{}` at {}:{}", *(key.value), key.span.line, key.span.col),
                            cerne::code_snippet(machine->code_sv, key.span, std::format("`{}` is not a valid path element.", *(key.value))),
                            key.span
                        );
                        machine->errors++;
                        return nullptr;
                    }

                    auto& equals = machine->peek(1);
                    if(equals.type != EQU) {
                        cerne::cerror(
                            machine->file_path,
                            ERR_UNEXPECTED_SYMBOL,
                            std::format("Unexpected token `{}` at {}:{}", *(equals.value), equals.span.line, equals.span.col),
                            cerne::code_snippet(machine->code_sv, equals.span, std::format("`{}` is not a valid path element.", *(equals.value))),
                            equals.span
                        );
                        machine->errors++;
                        return nullptr;
                    }

                    // now we advance past the identifier and the equals, and then we parse the node
                    machine->advance(2);
                    auto node = machine->parse(machine->peek());
                    if(!node) {
                        // break since we can't parse the current argument
                        break;
                    }

                    // initialize the initializer element and add it to the initializer data (and then continue to see if next is comma or not)
                    auto initializer_element = std::make_unique<cerne::InitializerElement>();
                    initializer_element->key = *(key.value);
                    initializer_element->key_span = key.span;
                    initializer_element->value = std::move(node);
                    initializer_data->values.push_back(std::move(initializer_element));
                    continue;
                } else if(token.type == COMMA) {
                    machine->advance();
                    continue;
                } else if(token.type == END_SCOPE) {
                    // break in end scope
                    break;
                } else {
                    // unkeyed, but we still check if initializer data is keyed since if it is, we need the name of the key to be present
                    if(initializer_data->is_keyed) {
                        cerne::cerror(
                            machine->file_path,
                            ERR_UNEXPECTED_SYMBOL,
                            std::format("Unexpected token `{}` at {}:{}", *(token.value), token.span.line, token.span.col),
                            cerne::code_snippet(machine->code_sv, token.span, std::format("`{}` is not a valid path element.", *(token.value))),
                            token.span
                        );
                        machine->errors++;
                        return nullptr;
                    }

                    // now we parse the node
                    auto node = machine->parse(machine->peek());
                    if(!node) {
                        // break since we can't parse the current argument
                        break;
                    }

                    // initialize the initializer element and add it to the initializer data (and then continue to see if next is comma or not)
                    auto initializer_element = std::make_unique<cerne::InitializerElement>();
                    initializer_element->value = std::move(node);
                    initializer_data->values.push_back(std::move(initializer_element));
                    continue;
                };
            }

            // check if we have a } to close the initializer
            if(machine->peek().type != END_SCOPE) {
                cerne::cerror(
                    machine->file_path,
                    ERR_UNEXPECTED_SYMBOL,
                    std::format("Unexpected token `{}` at {}:{}", *(token.value), token.span.line, token.span.col),
                    cerne::code_snippet(machine->code_sv, token.span, std::format("`{}` is not a valid path element.", *(token.value))),
                    token.span
                );
                machine->errors++;
                return nullptr;
            } else {
                // advance past the } and create the modifier based on the initializer data and add it to the modifiers of the element
                machine->advance();

                auto modifier = std::make_unique<cerne::Modifier>();
                modifier->type = cerne::ModifierTypes::INITIALIZER;
                modifier->data = std::move(initializer_data);
                element->modifiers.push_back(std::move(modifier));
            }

            // continue to the next possible modifier
            continue;
        }

        // <path, path, ...> (generic arguments)
        if(token.type == LESS_THAN) {
            // initialize generic arguments
            auto generic_args = std::vector<std::unique_ptr<cerne::Path>>();

            // collect the actual arguments
            while(machine->offset < machine->list.size()) {
                auto path = machine->parse_path(true);
                if(!path) {
                    // break since we can't parse the current argument
                    break;
                }

                // add path to generic args
                generic_args.push_back(std::move(path));

                // continue IF we have a , otherwise we break
                if(machine->peek().type == COMMA) {
                    machine->advance();
                    continue;
                } 

                break;
            }

            // continue to the next possible modifier
            continue;
        }

        // [node] (subscript)
        if(token.type == START_INDEX) {
            // since subscripts are modifiers that are not allowed in types, we set pure to false
            *pure = false;

            // initialize subscript data
            auto subscript_data = std::make_unique<cerne::SubscriptData>();

            // since subscripts only allow a single node, we simply parse the node and assign it to the subscript data
            auto node = machine->parse(machine->peek());
            if(!node) {
                // break since we can't parse the current argument
                break;
            }

            // assign the node to the subscript data
            subscript_data->index = std::move(node);

            // create the modifier based on the subscript data and add it to the modifiers of the element
            auto modifier = std::make_unique<cerne::Modifier>();
            modifier->type = cerne::ModifierTypes::SUBSCRIPT;
            modifier->data = std::move(subscript_data);
            element->modifiers.push_back(std::move(modifier));

            // continue to the next possible modifier
            continue;
        }

        // unexpected character in current element
        break;
    }

    return element;
}

/**
 * Utility to make creating simple types much faster
 */
std::unique_ptr<cerne::Path> cerne::create_simple_type(const std::string& name) {
    // type name in basicpathelement
    cerne::BasicPath path_elements;
    path_elements.reserve(1);
    auto element = cerne::BasicPathElement{
        .name=name,
        .is_member=false,
        .modifiers={}
    };
    path_elements.push_back(std::move(element));

    // create the path
    return std::make_unique<cerne::Path>(cerne::Path{
        .basic_path=std::move(path_elements),
        .pure_path=false
    });
}

/**
 * parses a path
 * is_type makes it so it can ONLY be a pure path (aka a type path)
 * this means parse_path will stop parsing as soon as it encounters a call or an initializer
 */
std::unique_ptr<cerne::Path> cerne::ParseMachine::parse_path(bool is_type) {
    auto path = std::make_unique<cerne::Path>();

    bool pure = true;

    // we parse path element until a path chain stopper is encountered (a token that cannot connect paths, aka a token that is not a member access or a dot)
    while(offset < list.size()) {
        auto element = parse_path_element(this, &pure);

        if(!element) break;

        path->basic_path.push_back(std::move(*element));

        auto& token = peek();

        if(token.type == MEMBER_ACCESS || token.type == DOT) {
            path->basic_path.back().is_member = (token.type == MEMBER_ACCESS);
            advance();
            continue;
        }

        break;
    }

    // if the path does not match with the pure path requirement, we throw an error
    if(is_type && !pure) {
        auto& token = peek();
        cerne::cerror(
            file_path,
            ERR_UNEXPECTED_SYMBOL,
            std::format("Unexpected token `{}` at {}:{}", *(token.value), token.span.line, token.span.col),
            cerne::code_snippet(code_sv, token.span, std::format("`{}` is not a valid path element.", *(token.value))),
            token.span
        );
        errors++;
        return nullptr;
    }

    // at last, after we go through the whole path, we determine whether it's pure or not from the variable
    path->pure_path = pure;

    return path;
}