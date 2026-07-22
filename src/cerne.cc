/*
    Cerne Compiler - Main Entry Point

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#include "include/cerne.hpp"

/**
 * separate utility to check ast print conditions
 */
void check_ast_print(const cerne::args& args, const cerne::AST* ast, bool error = false) {
    // check for print/dump flags and their values (to see if it's exclusively for AST)
    if(args.flags.contains("print_ast") || (args.flags.contains("print") && args.flags.at("print") == "ast")) {

        // push the JSON to debug_engine (or fallback to cout just in case debug_engine is null)
        auto json = cerne::json(ast, error);

        if(cerne::debug_engine) cerne::debug_engine->add_message(json);
        else std::cout << json << std::endl;

    } else if(args.flags.contains("dump_ast") || (args.flags.contains("dump") && args.flags.at("dump") == "ast")) {
        std::string dump_path = std::string(ast->file_path) + ".ast.json";
        std::ofstream dump_file(dump_path);
        dump_file << cerne::json(ast, error);
        dump_file.close();

        // add message to debug_engine (or fallback to cout just in case debug_engine is null)
        auto msg = std::format("{}{}• {}AST{}{}{} dumped to {}{}\n", BOLD, ce_colors::fgblue, UNDERLINE, RESET, BOLD, ce_colors::fgwhite, dump_path, RESET);

        if(cerne::debug_engine) cerne::debug_engine->add_message(msg);
        else std::cout << msg << std::endl;
    }
}

const uint8_t phases = 3; // lexing, parsing, sema (for now)

/**
 * for compilation diagnosis
 */
enum Phase {
    LEXING,
    PARSING,
    SEMANTIC_ANALYSIS
};

enum CompilationStatus {
    ERROR,
    WARNING,
    SUCCESS
};

struct Profile {
    std::array<double, phases> phase_times{0.0,0.0,0.0}; // lexing, parsing, sema
    double total_time{0.0};
};

struct Result {
    CompilationStatus status;
    std::vector<cerne::CERROR> errors;
    std::vector<std::string> messages;
    Profile profile;
};

/**
 * Actually runs and compiles the code
 * returns a Result struct containing:
 * - [enum CompilationStatus] status (ERROR|WARNING|SUCCESS)
 * - [vector<CERROR>] errors (if any)
 * - [vector<string>] messages (if any)
 * - [Profile] profile (contains phase times)
 */
Result compile(const cerne::args& args, const std::string& file, const std::string& code, std::atomic<Phase>& phase) {
    // first we immediately convert code to a string_view 
    const char* file_cstr = file.c_str();
    const auto& code_sv = std::string_view(code);
    auto status = CompilationStatus::SUCCESS;
    auto profile = Profile{};
    auto last_time = std::chrono::steady_clock::now();

    // for diagnostics we need to set the engine to a locally created engine class
    cerne::DEngine engine;
    cerne::diag_engine = &engine;

    // for debug as well
    cerne::DebugEngine debug_engine;
    cerne::debug_engine = &debug_engine;

    // now we pass it through the lexer
    auto [tokens, errors] = cerne::lexer(code_sv, file_cstr, args);

    // lexer is done so we can store the time for the lexing phase
    auto lexer_now = std::chrono::steady_clock::now();
    profile.phase_times[Phase::LEXING] = std::chrono::duration<double, std::milli>(lexer_now - last_time).count();
    profile.total_time += profile.phase_times[Phase::LEXING];
    last_time = lexer_now;

    // check if there are even any tokens and if not we just return "success" since there weren't any errors anyways
    if(tokens.size() == 0) {
        auto error_messages = cerne::diag_engine->get();
        auto messages = cerne::debug_engine->get();
        cerne::diag_engine = nullptr;
        cerne::debug_engine = nullptr;

        if(errors > 0) {
            return Result{CompilationStatus::ERROR, error_messages, messages, profile};
        } else {
            return Result{CompilationStatus::SUCCESS, {}, messages, profile};
        }
    }

    // update phase
    phase.store(Phase::PARSING);

    // after lexing, we pass the token list through the parser to generate an AST
    auto ast = cerne::parse(code_sv, tokens, file_cstr, args);

    // parser is done so we can store the time for the parsing phase
    auto parser_now = std::chrono::steady_clock::now();
    profile.phase_times[Phase::PARSING] = std::chrono::duration<double, std::milli>(parser_now - last_time).count();
    profile.total_time += profile.phase_times[Phase::PARSING];
    last_time = parser_now;

    // after parsing, we pass the AST through SEMA and then to IR generation, for now though, since those haven't been developed yet, the if statement will be blank
    if(ast->errors > 0) {
        check_ast_print(args, ast.get(), true);
        auto error_messages = cerne::diag_engine->get();
        auto messages = cerne::debug_engine->get();
        cerne::diag_engine = nullptr;
        cerne::debug_engine = nullptr;
        return Result{CompilationStatus::ERROR, error_messages, messages, profile};
    }

    status = (ast->warnings > 0) ? CompilationStatus::WARNING : CompilationStatus::SUCCESS;

    // begin SEMA now
    auto symbtable = std::make_unique<cerne::SymbolTable>(ast.get(), file_cstr, code_sv);
    symbtable->build();

    // SEMA is done so we can store the time for the SEMA phase
    auto sema_now = std::chrono::steady_clock::now();
    profile.phase_times[Phase::SEMANTIC_ANALYSIS] = std::chrono::duration<double, std::milli>(sema_now - last_time).count();
    profile.total_time += profile.phase_times[Phase::SEMANTIC_ANALYSIS];
    last_time = sema_now;

    // after going through symbol table construction of sema, check for errors
    if(ast->errors > 0) {
        check_ast_print(args, ast.get(), true);
        auto error_messages = cerne::diag_engine->get();
        auto messages = cerne::debug_engine->get();
        cerne::diag_engine = nullptr;
        cerne::debug_engine = nullptr;
        return Result{CompilationStatus::ERROR, error_messages, messages, profile};
    }
    
    status = (ast->warnings > 0) ? CompilationStatus::WARNING : CompilationStatus::SUCCESS;

    // ast diagnostics
    check_ast_print(args, ast.get());
    phase.store(Phase::SEMANTIC_ANALYSIS);
    
    // finish compilation and return the result
    auto error_messages = cerne::diag_engine->get();
    auto messages = cerne::debug_engine->get();
    cerne::diag_engine = nullptr;
    cerne::debug_engine = nullptr;
    return Result{status, error_messages, messages, profile};
}

/**
 * quickly turn the phase into a string for the loading message
 */
[[nodiscard]] std::string phase_to_str(const std::atomic<Phase>& phase) {
    switch(phase.load()) {
        case Phase::LEXING: return "Lexing";
        case Phase::PARSING: return "Parsing";
        case Phase::SEMANTIC_ANALYSIS: return "Analyzing";
        default: return "???";
    }
}

/**
 * Compilation pipeline
 */
void compile_files(const cerne::args& args, const std::vector<std::string>& files) {
    std::ranges::for_each(files, [&args](const std::string& file) {
        // read file and ignore if there's no code
        const auto& code = cerne::readf(file);
        
        if(!code.empty()) {
            std::atomic<Phase> phase = Phase::LEXING;
            auto compile_result = std::async(std::launch::async, compile, args, file, code, std::ref(phase));

            // loading message
            const uint8_t max_frames = 10;
            size_t frame_index = 0;
            const std::string frames[max_frames] = {"⠋","⠙","⠹","⠸","⠼","⠴","⠦","⠧","⠇","⠏"};
            while(compile_result.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
                std::string phase_message = std::format(
                    "{}{}{}{} {} {}",
                    BOLD,
                    ce_colors::fglblue,
                    frames[frame_index % 10],
                    RESET,
                    phase_to_str(phase),
                    file
                );

                std::cout << "\r" << phase_message << std::flush;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                frame_index++;
            }

            // get the result of the compilation and print the appropriate message
            auto result = compile_result.get();
            const auto time_template = std::format(
                "({}{}{}{}{}{}ms){}",
                BOLD,
                ce_colors::fgblue,
                result.profile.total_time,
                RESET,
                BOLD,
                ce_colors::fgwhite,
                RESET
            );

            auto status_color = ce_colors::fgwhite;
            switch(result.status) {
                case CompilationStatus::ERROR: {
                    std::cout << "\r" << BOLD << ce_colors::fgred << "✖" << RESET << 
                    BOLD << ce_colors::fgwhite << " Compilation failed with " << 
                    ce_colors::fgred << result.errors.size() << RESET << 
                    BOLD << ce_colors::fgwhite << " errors for " << file << ' ' << time_template << '\n' << std::flush;
                    status_color = ce_colors::fgred;
                    break;
                };

                case CompilationStatus::WARNING: {
                    std::cout << "\r" << BOLD << ce_colors::fgyellow << "⚠" << RESET << 
                    BOLD << ce_colors::fgwhite << " Compilation finished with " << 
                    ce_colors::fgyellow << result.errors.size() << RESET << 
                    BOLD << ce_colors::fgwhite << " warnings for " << file << ' ' << time_template << '\n' << std::flush;
                    status_color = ce_colors::fgyellow;
                    break;
                };

                case CompilationStatus::SUCCESS: {
                    std::cout << "\r" << BOLD << ce_colors::fggreen << "•" << RESET << 
                    BOLD << ce_colors::fgwhite << " Compilation finished successfully for " << file << ' ' << time_template << '\n' << std::flush;
                    status_color = ce_colors::fggreen;
                    break;
                };

                default:
                    break;
            }

            // print all errors
            cerne::DEngine::print_errors(result.errors);
            
            // print dump messages (after errors)
            cerne::DebugEngine::print_messages(result.messages);

            // check if the user wants the profile
            if(args.flags.contains("profile")) {
                // get all times
                const auto lexer_time = result.profile.phase_times[Phase::LEXING];
                const auto parser_time = result.profile.phase_times[Phase::PARSING];
                const auto sema_time = result.profile.phase_times[Phase::SEMANTIC_ANALYSIS];
                const auto total_time = result.profile.total_time;

                // percentages
                const auto lexer_percent = (lexer_time / total_time) * 100.0;
                const auto parser_percent = (parser_time / total_time) * 100.0;
                const auto sema_percent = (sema_time / total_time) * 100.0;

                // print
                std::cout << BOLD << ce_colors::fgwhite << "Profile for " << status_color << file << RESET << '\n' <<
                "───────────────────" << '\n' <<

                // lexer
                BOLD << ce_colors::fgwhite << "├─ Lexing: " << ce_colors::fgblue << lexer_time << RESET << BOLD << ce_colors::fgwhite << "ms (" << lexer_percent << "%)" << RESET << '\n' <<
                BOLD << ce_colors::fgwhite << "├─ Parsing: " << ce_colors::fgblue << parser_time << RESET << BOLD << ce_colors::fgwhite << "ms (" << parser_percent << "%)" << RESET << '\n' <<
                BOLD << ce_colors::fgwhite << "└─ SEMA: "   << ce_colors::fgblue << sema_time  << RESET << BOLD << ce_colors::fgwhite << "ms ("  << sema_percent << "%)" << RESET <<
                std::endl;
            }
        }
    });
}

int main(int argc, char** argv) {
    const auto& args = cerne::parse_args(argc, argv);
    auto cli = std::make_unique<cerne::CLI>(args);

    cli->event("files", [&args](const std::vector<std::string>& files) {
        compile_files(args, files);
    });

    cli->event("help", [&cli]() {
        cli->help();
    });

    cli->event("version", []() {
        std::cout << BOLD << ce_colors::fggreen << "🌱 Cerne " << ESC "[0m" << FG "255;1m" << "ー v" << CERNE_VERSION.alpha << "." << CERNE_VERSION.major << "." << CERNE_VERSION.minor << "\n" << RESET <<
        "├─ " << FG "255m" << "Made in C++20" << RESET << FG "255m" << "\n" <<
        "└─ " << FG "255m" << "Copyright (c) 2026 Cerne Project" << RESET << FG "255m" << "\n" <<
        RESET << std::endl;
    });

    return 0;
}