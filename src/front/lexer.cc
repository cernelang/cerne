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
        bool is_comment = false;
        bool is_mlc = false;
        bool is_string = false;
        std::string raw_str_content = "";

        LexerMachine(){}
        ~LexerMachine(){}

        void update(char c) {
            if(c == '\n') {
                line++;
                col=0;
                is_comment=false;
            } else col++;
        }
        void push(cerne::Token token) {
            tokens.push_back(token);
        }

        /**
         * 0 -> is not a comment
         * 1 -> is a comment/mlc and just skip
         * 2 -> is the end of an MLC and skip the next character
         */
        int comment(char c, char n) {
            if(is_mlc) {
                if(c == '*' && n == '/') {
                    is_mlc = false;
                    return 2;
                } else return 1;
            } else if(is_comment) return 1;

            return 0;
        }

        /**
         * won't track fstring and sstring metadata for now
         */
        bool string(char c) {
            if(is_string) {
                auto symb_type = symbols[c];
                if(symb_type == string_subtype) {
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

    for(size_t i = 0; i < code.size(); i++) {
        // store current character and next
        char c = code[i], n = 0;

        // update position and check for comments
        machine->update(c);
        int comment = machine->comment(c,n);
        if(comment) { i+=comment-1; continue; }

        // skip for strings since the function does it's needed job
        if(machine->string(c)) continue;

        if(isalpha(c)) {
            std::string word = "";

            while((isalnum(c) or c == '_') and i < code.size()) {
                word += c;
                machine->update(c);
                i++;
                c = code[i];
            }

            // the outer for loop will perform another i++ after this iteration ends, since it ends at the first non-variable character for word (which could be whitespace or a symbol), the outer i++ will skip that character, which can be valuable for a statement.
            i--;

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
        } else if(isdigit(c)) {

        } else {
            if(i + 1 < code.size()) n = code[i+1];
            std::string possible_conjecture{c,n};

            // check for symbols vs conjecture (conjecture is more prioritized than single symbol)
            if(conjectures.contains(possible_conjecture)) {
                // already update i to skip the next character since the next character is part of the conjecture
                i++;

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
                    case cerne::TokenTypes::STRING:
                    case cerne::TokenTypes::FSTRING:
                    case cerne::TokenTypes::SSTRING:
                        machine->is_string = true;
                        machine->string_subtype = symbol_type;
                        continue;

                    default:
                        auto token = cerne::Token{
                            .type=symbol_type,
                            .value=nullptr,
                            .line=machine->line,
                            .col=machine->col
                        };
                        machine->push(token);
                        continue;
                }
            } else if(!isspace(c)) {
                // unexpected symbol
                if(!options.contains("nodebug")) cerne::error(file_path, std::format("Unexpected symbol `{}` at {}:{}", c, machine->line, machine->col), "");
            }
        }
    }

    // save tokens before deleting the lexer machine and return them
    const auto& tokens = machine->tokens;
    return tokens;
}