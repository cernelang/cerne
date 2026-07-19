/*
    Cerne Compiler - Lexer Component, responsible for converting raw .ce source code into structured tokens.

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#include "../include/lexer.hpp"

/* --- GLOBAL MAPS --- */
const std::map<char, cerne::TokenTypes> symbols = {
    {'+', cerne::TokenTypes::PLUS},
    {'-', cerne::TokenTypes::MINUS},
    {'/', cerne::TokenTypes::DIV},
    {'*', cerne::TokenTypes::MUL},
    {'%', cerne::TokenTypes::MODULO},

    {'@', cerne::TokenTypes::ASM_STARTER},

    {'&', cerne::TokenTypes::BIT_AND},
    {'|', cerne::TokenTypes::BIT_OR},
    {'^', cerne::TokenTypes::BIT_XOR},
    {'~', cerne::TokenTypes::BIT_NOT},
    
    {'!', cerne::TokenTypes::NOT},

    {'<', cerne::TokenTypes::LESS_THAN},
    {'>', cerne::TokenTypes::GREATER_THAN},

    {',', cerne::TokenTypes::COMMA},
    {'.', cerne::TokenTypes::DOT},
    {';', cerne::TokenTypes::END},
    {'\n', cerne::TokenTypes::END},
    {'=', cerne::TokenTypes::EQU},
    {':', cerne::TokenTypes::DEFINE},

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

const std::map<std::string, cerne::TokenTypes, std::less<>> conjectures = {
    {"->", cerne::TokenTypes::ARROW},
    {"=>", cerne::TokenTypes::LAMBDA_ARROW},
    {"**", cerne::TokenTypes::POWER},
    {"..", cerne::TokenTypes::RANGE},
    {"::", cerne::TokenTypes::MEMBER_ACCESS},
    {"#!", cerne::TokenTypes::START_RULE },
    {"/*", cerne::TokenTypes::START_MLC},
    {"//", cerne::TokenTypes::START_COMMENT},
    {"<<", cerne::TokenTypes::LEFT_SHIFT},
    {">>", cerne::TokenTypes::RIGHT_SHIFT},
    {">=", cerne::TokenTypes::GREATER_EQUAL},
    {"<=", cerne::TokenTypes::LESS_EQUAL},
    {"==", cerne::TokenTypes::EQUAL},
    {"++", cerne::TokenTypes::INCREMENT},
    {"--", cerne::TokenTypes::DECREMENT},
    {"+=", cerne::TokenTypes::PLUS_EQU},
    {"-=", cerne::TokenTypes::MINUS_EQU},
    {"/=", cerne::TokenTypes::DIV_EQU},
    {"*=", cerne::TokenTypes::MUL_EQU},
    {"&=", cerne::TokenTypes::BIT_AND_EQU},
    {"|=", cerne::TokenTypes::BIT_OR_EQU},
    {"^=", cerne::TokenTypes::BIT_XOR_EQU},
    {"~=", cerne::TokenTypes::BIT_NOT_EQU},
    {"!=", cerne::TokenTypes::NOT_EQUAL},
    {"|>", cerne::TokenTypes::PIPELINE},
    {"||", cerne::TokenTypes::OR},
    {"&&", cerne::TokenTypes::AND}
};

const std::map<std::string, cerne::TokenTypes, std::less<>> compounds = {
    {"<<=", cerne::TokenTypes::LEFT_SHIFT_EQU},
    {">>=", cerne::TokenTypes::RIGHT_SHIFT_EQU},
    {"...", cerne::TokenTypes::UNPACK}
};

/* --- LEXER MACHINE METHODS --- */

void cerne::LexerMachine::update(char c) {
    if(c == '\n') {
        line++;
        col=0;
        is_comment=false;
    } else col++;
}

void cerne::LexerMachine::push(cerne::Token&& token) {
    tokens.push_back(std::move(token));
}

/**
 * is a comment or not
 */
bool cerne::LexerMachine::comment(char c, char n) {
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
bool cerne::LexerMachine::string(char c) {
    if(is_string) {
        auto it = symbols.find(c);
        if(it != symbols.end() && it->second == string_subtype) {
            // create token for the string, push it and reset machine's string information for the next string
            auto token = cerne::Token(
                string_subtype,
                std::make_unique<std::string>(raw_str_content),
                cerne::Span{
                    .line=line,
                    .col=col-raw_str_content.size(),
                    .offset=offset,
                    .length=raw_str_content.size()
                }
            );
            push(std::move(token));
            is_string = false;
            raw_str_content.clear();
            raw_str_content.shrink_to_fit();
        } else raw_str_content += c;

        return true;
    }

    return false;
}

void cerne::LexerMachine::word(char c) {
    // snatch the whole word (loop through ever character next to this one until it's not alnum or an underscore)
    std::string word = "";
    size_t len = 1;

    while((isalnum(c) || c == '_') && offset < code.size()) {
        word += c;
        update(c);
        offset++;
        len++;
        c = code[offset];
    }

    // the outer for loop will perform another offset++ after this iteration ends, since it ends at the first non-variable character for word (which could be whitespace or a symbol), the outer offset++ will skip that character, which can be valuable for a statement.
    offset--;
    col--;
    len--;

    // to get the type, we check if the word is a keyword or register and if it is, then we set the type to that
    auto type = cerne::TokenTypes::IDENTIFIER;

    if(std::ranges::find(cerne::keywords, word) != cerne::keywords.end()) type = cerne::TokenTypes::MNEMONIC;
    else if(std::ranges::find(cerne::registers, word) != cerne::registers.end()) type = cerne::TokenTypes::REGISTER;

    // now push the token to our machine's token list
    auto token = cerne::Token(
        type,
        std::make_unique<std::string>(word),
        cerne::Span{
            .line=line,
            .col=(col >= len ? col-len : 0),
            .offset=(offset >= len ? offset-len : 0),
            .length=len
        }
    );
    push(std::move(token));
}

/**
 * numbers can come in various formats, mainly:
 * 1000
 * 1,000.0
 * 1e3
 * 0x3e8
 * among others
 */
void cerne::LexerMachine::number(char c) {
    // for numbers, it's actually more convenient to store them as strings as well instead of immediately converting to long/double
    std::string number = number_began_in_dot ? "0." : ""; // we put a 0. so that, in the future, when parsing this number, we correctly parse .5 as 0.5 instead
    size_t len = 1;
    bool error = false;
    size_t error_at = 0;
    uint8_t dots = number_began_in_dot;

    // reset to false
    number_began_in_dot = false;

    while((isdigit(c) || c == ',' || c == '.') && offset < code.size()) {
        // if there already is a dot, we're probably dealing with a range, so break loop
        if(c == '.' && (number.find('.') != std::string::npos)) {
            // it's a range
            if(number[number.size()-1] == '.' && dots <= 2) {
                number.pop_back();
                error=false;
                // decrement offset because we're trying to parse the range, which requires us to go back to the first dot
                offset--;
                break;
            } 

            // it's an error! too many dots in a number
            else if(!error) {
                error=true;
                error_at=len-1;
            }
        }

        // increment amount of dots
        if(c == '.') dots++;

        // , are just for style, they don't add any additional information to the number
        if(c != ',') number += c;
        update(c);
        offset++;
        len++;
        c = code[offset];
    }

    // realign offset (explanation on another comment in the word check (word() function), since the number loop mechanism is the same)
    offset--;
    col--;
    len--;

    const auto& span = cerne::Span{
        .line=line,
        .col=(col >= len ? col-len : 0),
        .offset=(offset >= len ? offset-len : 0),
        .length=len
    };

    // display dots error message
    if(error) {
        const auto& err_at_span = cerne::Span{.line=span.line,.col=span.col+error_at,.offset=span.offset+error_at,.length=span.length};

        cerne::cerror(
            file_path,
            ERR_TOO_MANY_DOTS,
            "Too many dots in number",
            cerne::code_snippet(
                code,
                err_at_span,
                std::format("`{}` is not a valid number", number.substr(0, 32))
            ),
            err_at_span
        );
        return;
    }

    // push number token to the machine's token list
    auto token = cerne::Token(
        cerne::TokenTypes::NUMBER,
        std::make_unique<std::string>(number),
        span
    );

    push(std::move(token));
}

/**
 * triple character symbols
 */
void cerne::LexerMachine::compound(const std::string& possible_compound) {
    // update to over after the last character of the compound
    offset += 2;
    
    // get compound type and then
    // check if the compound type is unpack and after unpack is a number, since it would mean it's a number range number (.5 is a number, 0...5 is a range between 0 and 0.5)
    if(auto compound_type = compounds.at(possible_compound); !(compound_type == cerne::TokenTypes::UNPACK && (offset+1 < code.length()) && isdigit(code[offset + 1]))) {

        auto token = cerne::Token(
            compound_type,
            std::make_unique<std::string>(possible_compound),
            cerne::Span{
                .line=line,
                .col=(col >= 3 ? col-3 : 0),
                .offset=(offset >= 3 ? offset-3 : 0),
                .length=3
            }
        );
        
        // push compound and skip over the third character as well
        push(std::move(token));
        return;
        
    }

    // if the condition above is not met, it's definitely a number range number situation
    auto range = cerne::Token(
        cerne::TokenTypes::RANGE,
        std::make_unique<std::string>(possible_compound),
        cerne::Span{
            .line=line,
            .col=(col >= 3 ? col-3 : 0),
            .offset=(offset >= 3 ? offset-3 : 0),
            .length=3
        }
    );

    // set number_began_in_dot to true and reset offset back so number parses the first dot correctly
    number_began_in_dot = true;
    push(std::move(range));
    return;
}

/**
 * double character symbols
 */
void cerne::LexerMachine::conjecture(const std::string& possible_conjecture) {
    // already update the offset to skip the next character since the next character is part of the conjecture
    offset++;

    // get the conjecture type and in case it's a comment, set the respective comment type to true in the machine
    auto conjecture_type = conjectures.at(possible_conjecture);
    switch(conjecture_type) {
        // one liner
        case cerne::TokenTypes::START_COMMENT:
            is_comment = true;
            break;
        
            // multiple line
        case cerne::TokenTypes::START_MLC:
            is_mlc = true;
            break;

        // not a comment? push the token for the conjecture
        default:
            auto token = cerne::Token(
                conjecture_type,
                std::make_unique<std::string>(possible_conjecture),
                cerne::Span{
                    .line=line,
                    .col=(col >= 1 ? col-1 : 0),
                    .offset=(offset >= 1 ? offset-1 : 0),
                    .length=2
                }
            );
            push(std::move(token));
    }
}

/**
 * single character symbols
 */
void cerne::LexerMachine::symbol(char c, char n) {
    auto symbol_type = symbols.at(c);

    switch(symbol_type) {
        // activate string collection (with subtype for metadata)
        case TokenTypes::STRING:
        case TokenTypes::FSTRING:
        case TokenTypes::SSTRING:
            is_string = true;
            string_subtype = symbol_type;
            break;

        case TokenTypes::DOT:
            // if next is a number, it's a literal like .5
            if(isdigit(n)) {
                number_began_in_dot = true;
            } else {
                auto dot = cerne::Token(
                    symbol_type,
                    std::make_unique<std::string>(std::string{c}),
                    cerne::Span{
                        .line=line,
                        .col=(col >= 1 ? col-1 : 0),
                        .offset=(offset >= 1 ? offset-1 : 0),
                        .length=1
                    }
                );
                push(std::move(dot));
            }

            break;  // we just break and let number handle it naturally, since the dot is part of the number

        default:
            // for any other symbol, just push to the token list
            auto token = cerne::Token(
                symbol_type,
                std::make_unique<std::string>(std::string{c}),
                cerne::Span{
                    .line=line,
                    .col=(col >= 1 ? col-1 : 0),
                    .offset=(offset >= 1 ? offset-1 : 0),
                    .length=1
                }
            );
            push(std::move(token));
    }
}

/**
 * Goes through each character and converts into it's appropriate token.
 */
std::vector<cerne::Token> cerne::lexer(const std::string_view& code, const char* file_path, const cerne::args& options) {
    auto machine = std::make_unique<LexerMachine>(code, file_path, options);

    for(; machine->offset < code.size(); machine->offset++) {
        // store current character and next
        char c = code[machine->offset], n = 0;
        if(machine->offset + 1 < code.size()) n = code[machine->offset+1];

        // update position and check for comments
        machine->update(c);
        if(machine->comment(c,n)) continue;

        // skip for strings since the function does it's needed job
        if(machine->string(c)) continue;

        // word checking
        if(isalpha(c) || c == '_') {
            machine->word(c);
        } 
        // number checking
        else if(isdigit(c)) {
            machine->number(c);
        } 
        // none of those? it's a symbol/whitespace
        else {
            // nn only here for performance
            char nn = 0;
            if(machine->offset + 2 < code.size()) nn = code[machine->offset+2];

            // conjectures - 2 letter operations, compounds - 3 letter operations while symbols are only 1 character long
            std::string possible_conjecture{c,n};
            std::string possible_compound{c,n,nn};

            // check for symbols vs conjecture vs compounds (priority -> compounds > conjectures > symbols)
            if(compounds.contains(possible_compound)) {
                machine->compound(possible_compound);
                continue;
            } else if(conjectures.contains(possible_conjecture)) {
                machine->conjecture(possible_conjecture);
                continue;
            } else if(symbols.contains(c)) {
                machine->symbol(c, n);
                continue;
            } 
            // unexpected character (should ignore whitespaces)
            else if(!isspace(c) && !options.flags.contains("quiet")) { 
                const auto& _cerr_span = cerne::Span{
                    .line=machine->line,
                    .col=machine->col,
                    .offset=machine->offset,
                    .length=1
                };

                cerne::cerror(
                    file_path, 
                    ERR_UNEXPECTED_SYMBOL,
                    std::format("Unexpected symbol `{}` at {}:{}", c, machine->line, machine->col), 
                    cerne::code_snippet(code, _cerr_span, "Unexpected symbol here"),
                    _cerr_span
                );
                return {};
            }
        }
    }

    // Move tokens out of the unique_ptr to avoid trying to copy non-copyable Token structs
    auto tokens = std::move(machine->tokens);
    return tokens;
}