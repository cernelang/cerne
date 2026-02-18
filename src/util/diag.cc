/*
    Cerne Compiler - Utility for Cerne's logging needs

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#include "../include/utils.hpp"
#include "../include/lexer.hpp"

/**
 * General error logging
 */
void cerne::error(const char* src, const std::string& message) {
    // print error message
    std::cerr << FG "196;1m" << SIGNATURE << 
    ' ' << src << FG "255m" << ':' << ' ' << message <<  
    RESET << std::endl;
}

/**
 * This function is dedicated to log compiler errors
 */
void cerne::cerror(const char* src, const size_t& errcode, const std::string_view& message, const std::string_view& code_snippet, const cerne::Span& span) {
    // print appropriate error message
    size_t digits = std::log10(errcode) + 1;

    std::cerr << FG "196;1m" << "[CNE" << std::string(4-digits, '0') << errcode << "]" << 
    FG "255m" << ':' << ' ' << message << '\n' << '\n' <<
    FG "219;1m" << src << ':' << span.line << ":" << span.col << FG "255m" << '\n' <<
    code_snippet << 
    RESET << std::endl;
}

// code snippet & helper
/**
 * To prevent printing huge lines, we use col as well to see where the error is and only print
 * a window of the line around the error.
 * Maximum line size displayed is 80 characters.
 */
std::string get_line(const std::string& code, size_t line, size_t col) {
    size_t current_line = 1;
    size_t line_start = 0;

    // find the start of the line
    while(current_line < line && line_start < code.size()) {
        if(code[line_start] == '\n') current_line++;
        line_start++;
    }

    // if we reached EOF, return empty string
    if(line_start >= code.size()) return "";

    // find the end of the line
    size_t line_end = line_start;
    while(line_end < code.size() && code[line_end] != '\n') line_end++;

    size_t line_length = line_end - line_start;

    // now we have the start and end of the line, we can calculate the window around the error
    size_t window_start = (col > 40) ? std::max(col - 44, (size_t)0) : 0;
    size_t window_end = std::min(col + 36, line_length);

    auto substr = code.substr(line_start + window_start, window_end - window_start);

    if(window_end < line_length) {
        substr += " ...";
    } 

    if(window_start > 0) {
        substr.insert(0, "... ");
    }

    return substr;
}

std::string_view cerne::code_snippet(const std::string_view& code, cerne::Span span, const std::string_view& under_message) {
    static thread_local std::string window;
    window.clear();
    
    // we first snatch the line of the error 
    std::string line = get_line(std::string(code), span.line, span.col);

    // line 400 is 3 characters wide for example while line 50000 is 5 characters wide, so depending
    // on line number, the padding will change
    size_t width = std::log10(span.line) + 1;

    // print lines
    window += std::string(width+1, ' ') + FG "255;1m" + '|' + '\n';
    window += FG "255m" + std::to_string(span.line) + ' ' + '|' + '\t' + std::string(line) + '\n';
    window += std::string(width+1, ' ') + FG "255;1m" + '|' + '\t' + std::string(((line.size()==80)?4:(span.col%80)), ' ') + FG "196m" + '^' + std::string(std::min(span.length - 1, (size_t)(line.size()-(span.col%40))), '~') + '\n' + RESET;
    window += std::string(width+1, ' ') + FG "255;1m" + '|' + '\t' + std::string(((line.size()==80)?4:(span.col%80)), ' ') + std::string(under_message) + '\n';

    return std::string_view(window);
}

// log with a time
void cerne::tlog(double time, const std::string& message) {
    std::cout << FG "111;1m" SIGNATURE << FG "237m" " [" << FG "75m" << time << FG "237m" "]" << FG "255m: " << message << std::endl;
}

// standard debug function
void cerne::debug(const std::string_view& message) {
    std::cout << FG "111;1m" SIGNATURE <<  FG "237m" " [" << FG "75m" << "debug" << FG "237m" "]" << FG "255m: " << message << RESET << std::endl;
}

// special expected function
void cerne::expected(const char* src, const cerne::Span& span, const std::string_view& expected, const std::string_view& got) {
    cerne::cerror(
        src, 
        2,
        std::format("Expected {}, instead got {} at {}:{}", expected, got, span.line, span.col), 
        "",
        span
    );
}