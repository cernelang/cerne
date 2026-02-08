/*
    Cerne Compiler - Lexer Component, responsible for converting raw .ce source code into structured tokens.

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#include "../include/lexer.hpp"

std::map<char, cerne::TokenTypes> symbols = {
    {'+', cerne::TokenTypes::PLUS},
    {'-', cerne::TokenTypes::MINUS},
    {'/', cerne::TokenTypes::DIV},
    {'*', cerne::TokenTypes::MUL},
    {'.', cerne::TokenTypes::DOT},
    {'"', cerne::TokenTypes::STRING},
    {'\'', cerne::TokenTypes::FSTRING},
    {'`', cerne::TokenTypes::SSTRING},
    {'(', cerne::TokenTypes::START_PARAM},
    {')', cerne::TokenTypes::END_PARAM},
    {'[', cerne::TokenTypes::START_INDEX},
    {']', cerne::TokenTypes::END_INDEX},
    {'{', cerne::TokenTypes::START_SCOPE},
    {'}', cerne::TokenTypes::END_SCOPE}
};

std::map<std::string, cerne::TokenTypes> conjectures = {
    {"->", cerne::TokenTypes::ARROW},
    {"/*", cerne::TokenTypes::START_MLC},
    {"//", cerne::TokenTypes::START_COMMENT}
};

class LexerMachine {
    public:
        std::vector<cerne::Token> tokens;
        cerne::TokenTypes string_subtype;
        size_t line = 1;
        size_t col = 0;
        size_t offset = 0;
        bool is_comment = false;
        bool is_mlc = false;
        bool is_string = false;
        std::string raw_str_content = "";

        LexerMachine()=default;
        ~LexerMachine()=default;

        void update(char c) {
            if(c == '\n') {
                line++;
                col=0;
                is_comment=false;
            } else col++;
        }
        void push(const cerne::Token& token) {
            tokens.push_back(token);
        }

        /**
         * is a comment or not
         */
        bool comment(char c, char n) {
            if(is_mlc) {
                if(c == '*' && n == '/') {
                    is_mlc = false;
                    offset++;
                    return true;
                } else return true;
            } else if(is_comment) return true;

            return false;
        }

        /**
         * won't track fstring and sstring metadata for now
         */
        bool string(char c) {
            if(is_string) {
                if(symbols[c] == string_subtype) {
                    // create token for the string, push it and reset machine's string information for the next string
                    auto token = cerne::Token{
                        .type=string_subtype,
                        .value=(void*)(new std::string(raw_str_content)),
                        .line=line,
                        .col=col
                    };
                    push(token);
                    is_string = false;
                    raw_str_content = "";
                } else raw_str_content += c;

                return true;
            }

            return false;
        }
};

/**
 * Goes through each character and converts into it's appropriate token.
 */
std::vector<cerne::Token> cerne::lexer(const std::string_view& code, const char* file_path, const cerne::args& options) {
    auto machine = std::make_unique<LexerMachine>();

    for(; machine->offset < code.size(); machine->offset++) {
        // store current character and next
        char c = code[machine->offset], n = 0;

        // update position and check for comments
        machine->update(c);
        if(machine->comment(c,n)) continue;

        // skip for strings since the function does it's needed job
        if(machine->string(c)) continue;

        // word checking
        if(isalpha(c) || c == '_') {
            // snatch the whole word (loop through ever character next to this one until it's not alnum or an underscore)
            std::string word = "";

            while((isalnum(c) || c == '_') && machine->offset < code.size()) {
                word += c;
                machine->update(c);
                machine->offset++;
                c = code[machine->offset];
            }

            // the outer for loop will perform another offset++ after this iteration ends, since it ends at the first non-variable character for word (which could be whitespace or a symbol), the outer offset++ will skip that character, which can be valuable for a statement.
            machine->offset--;

            // to get the type, we first check if the word is in keywords, then registers, and if it's in neither then it's an IDENTIFIER
            auto type = 
                (std::find(cerne::keywords.begin(), cerne::keywords.end(), word) != cerne::keywords.end()) ? cerne::TokenTypes::MNEMONIC 
                : (std::find(cerne::registers.begin(), cerne::registers.end(), word) != cerne::registers.end()) ? cerne::TokenTypes::REGISTER 
                : cerne::TokenTypes::IDENTIFIER;

            // now push the token to our machine's token list
            auto token = cerne::Token{
                .type=type,
                .value=(void*)(new std::string(word)),
                .line=machine->line,
                .col=machine->col
            };
            machine->push(token);
        } 
        // number checking
        else if(isdigit(c)) {
            // for numbers, it's actually more convenient to store them as strings as well instead of immediately converting to long/double
            std::string number = "";

            while((isdigit(c) || c == ',' || c == '.') && machine->offset < code.size()) {
                // if there already is a dot, error since multiple dots in a number is invalid
                if(number.find('.') != number.npos) {
                    cerne::error(file_path, "Multiple dots in number", "");
                }

                // , are just for style, they don't add any additional information to the number
                if(c != ',') number += c;
                machine->update(c);
                machine->offset++;
                c = code[machine->offset];
            }

            // realign offset (explanation on another comment in the word check, since the number loop mechanism is the same)
            machine->offset--;

            // push number token to the machine's token list
            auto token = cerne::Token{
                .type=cerne::TokenTypes::NUMBER,
                .value=(void*)(new std::string(number)),
                .line=machine->line,
                .col=machine->col
            };

            machine->push(token);
        } 
        // none of those? it's a symbol/whitespace
        else {
            if(machine->offset + 1 < code.size()) n = code[machine->offset+1];
            std::string possible_conjecture{c,n};

            // check for symbols vs conjecture (conjecture is more prioritized than single symbol)
            if(conjectures.contains(possible_conjecture)) {
                // already update the offset to skip the next character since the next character is part of the conjecture
                machine->offset++;

                // get the conjecture type and in case it's a comment, set the respective comment type to true in the machine
                auto conjecture_type = conjectures[possible_conjecture];
                switch(conjecture_type) {
                    // one liner
                    case cerne::TokenTypes::START_COMMENT:
                        machine->is_comment = true;
                        continue;
                    
                        // multiple line
                    case cerne::TokenTypes::START_MLC:
                        machine->is_mlc = true;
                        continue;

                    // not a comment? push the token for the conjecture
                    default:
                        auto token = cerne::Token{
                            .type=conjecture_type,
                            .value=nullptr,
                            .line=machine->line,
                            .col=machine->col
                        };
                        machine->push(token);
                        continue;
                }
            } else if(symbols.contains(c)) {
                auto symbol_type = symbols[c];

                switch(symbol_type) {
                    // activate string collection (with subtype for metadata)
                    case cerne::TokenTypes::STRING:
                    case cerne::TokenTypes::FSTRING:
                    case cerne::TokenTypes::SSTRING:
                        machine->is_string = true;
                        machine->string_subtype = symbol_type;
                        continue;

                    default:
                        // for any other symbol, just push to the token list
                        auto token = cerne::Token{
                            .type=symbol_type,
                            .value=nullptr,
                            .line=machine->line,
                            .col=machine->col
                        };
                        machine->push(token);
                        continue;
                }
            } 
            // (should ignore whitespaces)
            else if(!isspace(c)) { 
                // unexpected symbol
                if(!options.contains("nodebug")) cerne::error(file_path, std::format("Unexpected symbol `{}` at {}:{}", c, machine->line, machine->col), "");
            }
        }
    }

    // save tokens before deleting the lexer machine and return them
    const auto& tokens = machine->tokens;
    return tokens;
}