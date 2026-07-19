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
void cerne::cerror(
        const char* src, 
        const size_t& errcode, 
        const std::string_view& message, 
        const std::string_view& code_snippet, 
        const cerne::Span& span,
        const std::string_view& extras
    ) {
    // print appropriate error message
    std::cerr << 
    std::format(
        "{}[{}{}{}]{}", 
        FG "196;1m", 
        std::format("{}]8;;https://docs.cerne.space/errors#code_{}{}", ESC, errcode, "\x07"),
        std::format("CNE{:04d}", errcode),
        ESC "]8;;" "\x07" RESET FG "196;1m", RESET BOLD
    ) << 
    FG "255m" << ':' << ' ' << message << '\n' << '\n' <<
    FG "219;1m" << src << ':' << span.line << ":" << span.col << FG "255m" << '\n' << RESET <<
    code_snippet << 
    RESET << 
    extras << 
    RESET <<
    std::endl;
}

// code snippet & helper
struct LineWindow {
    std::string line;
    size_t start;
};

/**
 * To prevent printing huge lines, we use col as well to see where the error is and only print
 * a window of the line around the error.
 * Maximum line size displayed is 80 characters.
 */
LineWindow get_line(const std::string& code, size_t line, size_t col) {
    size_t current_line = 1;
    size_t line_start = 0;

    // find the start of the line
    while(current_line < line && line_start < code.size()) {
        if(code[line_start] == '\n') current_line++;
        line_start++;
    }

    // if we reached EOF, return empty string
    if(line_start >= code.size()) return LineWindow{.line="", .start=0};

    // find the end of the line
    size_t line_end = line_start;
    while(line_end < code.size() && code[line_end] != '\n') line_end++;

    size_t line_length = line_end - line_start;

    // now we have the start and end of the line, we can calculate the window around the error
    col = std::min(col, line_length); // ensure col is within the line length

    size_t window_start = (col >= 44) ? col - 44 : 0;
    size_t window_end = std::min(col + 36, line_length);

    auto substr = code.substr(line_start + window_start, window_end - window_start);

    if(window_end < line_length) {
        substr += " ...";
    } 

    if(window_start > 0) {
        substr.insert(0, "... ");
    }

    return LineWindow{.line=substr,.start=window_start};
}

std::string_view cerne::code_snippet(const std::string_view& code, cerne::Span span, const std::string_view& under_message) {
    static thread_local std::string window;
    window.clear();
    
    // we first snatch the line of the error 
    LineWindow line_window = get_line(std::string(code), span.line, span.col);
    std::string line = line_window.line;
    size_t start = line_window.start;

    // line 400 is 3 characters wide for example while line 50000 is 5 characters wide, so depending
    // on line number, the padding will change
    size_t width = std::log10(span.line) + 1;

    // print lines
    window += std::format("{}{}|\n", ce_colors::fggray, std::string(width+1, ' '));
    window += std::format("{}{} |{}{}\t{}\n{}", ce_colors::fggray, std::to_string(span.line), RESET, ce_colors::fgwhite, line, RESET);
    window += std::format("{}{}|{}{}\t{}\n", ce_colors::fggray, std::string(width+1, ' '), RESET BOLD, ce_colors::fgwhite, std::string(span.col - start, ' ') + ce_colors::fgred + '^' + std::string(std::min(span.length - 1, (size_t)(line.size()-(span.col%40))), '~') + RESET);
    window += std::format("{}{}|{}{}\t{}\n{}",ce_colors::fggray, std::string(width+1, ' '), RESET BOLD, ce_colors::fgwhite, std::string(span.col - start, ' ') + std::string(under_message), RESET);

    return std::string_view(window);
}

std::string cerne::oneline_code_snippet(const std::string_view& code, cerne::Span span) {
    LineWindow line_window = get_line(std::string(code), span.line, span.col);
    std::string line = line_window.line;
    std::string window = std::format("{}{}{} |{}{}\t{}{}", RESET, ce_colors::fggray, std::to_string(span.line), RESET, ce_colors::fgwhite, line, RESET);
    return window;
}

// log with a time
void cerne::tlog(double time, const std::string& message) {
    std::cout << FG "111;1m" SIGNATURE << FG "237m" " [" << FG "75m" << time << FG "237m" "]" << FG "255m: " << message << std::endl;
}

// standard debug function
void cerne::debug(const std::string_view& message) {
    std::cout << FG "111;1m" SIGNATURE <<  FG "237m" " [" << FG "75m" << "debug" << FG "237m" "]" << FG "255m: " << message << RESET << std::endl;
}

/**
 * Utility for syntax highlighting
 * this makes cerne code in diagnostics look much better and more readable since you can see the distinction between tokens.
 */
std::string highlight(const std::string& code) {



    return code; // temporary
}

/**
 * Utility for simply converting notes into a more consistent format (used for footers in diagnostics or manual notes for example)
 */
std::string cerne::note(const std::string& base_note) {
    return std::format("\n{}{}= Note{}{}: {}{}\n", BOLD, ce_colors::fgblue, RESET BOLD, ce_colors::fgwhite, base_note, RESET);
}

/**
 * Same as before but for examples
 */
std::string cerne::example(const std::string& base_example) {
    return std::format("\n{}{}= Example{}{}: {}{}\n", BOLD, ce_colors::fggreen, RESET BOLD, ce_colors::fgwhite, base_example, RESET);
}

std::string cerne::help(const std::string& base_note) {
    return std::format("\n{}{}help{}{}: {}{}\n", BOLD, ce_colors::fgyellow, RESET BOLD, ce_colors::fgwhite, base_note, RESET);
}