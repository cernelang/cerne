/*
    Cerne Compiler - Main utility for the CLI interface.

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#include "../include/utils.hpp"

// global commands list
const std::vector<std::string> commands = {"help", "version"};

// --- methods ---
/**
 * Responsible for creating a flag (which is literally just a pair of strings) containing
 * the name of the flag and it's value
 */
cerne::flag parse_flag(const std::string_view& arg, const std::string_view& next) {
    // how much to remove from the flag
    size_t rem = 1;

    if(arg.size() > 1 && arg.at(1) == '-') rem++;

    // --flag=something
    const size_t& pos_equ = arg.find("=");
    if(pos_equ != std::string_view::npos) {
        // get name and value of the flag
        const std::string_view& name    = arg.substr(rem, pos_equ-rem);
        const std::string_view& value   = arg.substr(pos_equ + 1);

        return cerne::flag(std::string(name), std::string(value));
    } 
    else if(next.size() > 0) return cerne::flag(std::string(arg.substr(rem)), std::string(next));
    else return cerne::flag(std::string(arg.substr(rem)), "");
}

/**
 * Responsible for parsing the arguments from the command line
 * and returning a struct containing the flags and files
 */
cerne::args cerne::parse_args(int argc, char** argv) {
    cerne::args args;

    for(int i = 1; i < argc; i++) {
        std::string arg(argv[i]), next("");

        if(i + 1 < argc) next = std::string(argv[i+1]);

        if(arg[0] == '-') {
            // initialize next_arg
            auto next_arg = std::string_view(next);
            
            // if the next argument is a flag, we want to ignore it as a value
            // else, we want the next argument to BE the value and we want to skip it in the next iteration
            if(!next.empty() && next.at(0) == '-') {
                next_arg = std::string_view("");
            } else if(!next.empty()) i++;

            // actual flag
            cerne::flag _flag = parse_flag(std::string_view(arg), next_arg);
            args.flags.insert(_flag);
        } else {
            // if not we check if it's a command and if not we just push it as a file
            if(std::ranges::find(commands, arg) != commands.end()) {
                auto _flag = std::pair<std::string, std::string>(arg, "");
                args.flags.insert(_flag);
                continue;
            }
            
            args.files.push_back(arg);
        }
    }

    // no flags or files? then print the help function
    if(args.flags.empty() && args.files.empty()) args.flags.try_emplace("help", "");

    return args;
}

/**
 * Handles the events for the CLI
 * taking the type of event (could be a command or "files" to get the files) and calling the appropriate callback function
 */
void cerne::CLI::event(const std::string& type, cerne::callback fnc) const {
    const auto& args = this->__args;
    const auto& flags = args.flags;
    const auto& files = args.files;

    std::visit([&flags, &files, &type](auto&& new_fnc) {
        using fnc_stripped = std::decay_t<decltype(new_fnc)>;

        if constexpr (std::is_invocable_v<fnc_stripped, std::vector<std::string>>) {
            new_fnc(files);
        } else {
            if(flags.contains(type)) new_fnc();
        }
    }, fnc);
}

/**
 * Provides a helpful message for the users who are unsure of how to use the CLI or any of the tools
 * or just want to know more about the compiler in general.
 */
void cerne::CLI::help() const {
    // first line
    std::cout << BOLD << ce_colors::fggreen << "🌱 Cerne " << RESET << BOLD << ce_colors::fgwhite << "ー v" << CERNE_VERSION.alpha << "." << CERNE_VERSION.major << "." << CERNE_VERSION.minor << "\n" <<
    BOLD << ce_colors::fgwhite << "   Welcome to Cerne! Please look below to see all commands, flags and how to use them for your purpose.\n\n" << RESET << // 3 spaces to line up with the emoji
    
    // command usage just in case the user doesn't know how to use the CLI or just wants a reminder
    BOLD << ce_colors::fgblue << "◆ USAGE" << BOLD << ce_colors::fgwhite << ":\n" << RESET <<
    "├─\t" << BOLD << ce_colors::fggreen << "cerne " << RESET << BOLD << ce_colors::fgwhite << "<command> [options]\n" <<
    "└─\t" << BOLD << ce_colors::fggreen << "cerne " << RESET << BOLD << ce_colors::fgwhite << "<files> [flags]\n\n" <<
    
    // command list in the help message
    BOLD << ce_colors::fgblue << "◆ COMMANDS" << BOLD << ce_colors::fgwhite << ":\n" << RESET <<
    "├─\t" << BOLD << ce_colors::fgwhite << "help" << RESET<< ce_colors::fgwhite << "\t\t\t shows this message\n" <<
    "└─\t" << BOLD << ce_colors::fgwhite << "version" << RESET<< ce_colors::fgwhite << "\t\t\t shows the current version of the compiler\n\n" <<

    // possible flags
    BOLD << ce_colors::fgblue << "◆ FLAGS" << BOLD << ce_colors::fgwhite << ":\n" << RESET <<
    "├─\t" << ce_colors::fgwhite << "-h, --help" << RESET << ce_colors::fgwhite << "\t\t shows this message\n" <<
    "├─\t" << ce_colors::fgwhite << "-v, --version" << RESET << ce_colors::fgwhite << "\t\t shows the current version of the compiler\n" <<
    "├─\t" << ce_colors::fgwhite << "--debug" << RESET << ce_colors::fgwhite << "\t\t\t shows debug information\n" <<
    "├─\t" << ce_colors::fgwhite << "--dump=<option>" << RESET << ce_colors::fgwhite << "\t\t dumps the specified option (options: " << ce_colors::bgblack << "ast" << RESET << ce_colors::fgwhite << ")\n" <<
    "├─\t" << ce_colors::fgwhite << "--print=<option>" << RESET << ce_colors::fgwhite << "\t prints the specified option (options: " << ce_colors::bgblack << "ast" << RESET << ce_colors::fgwhite << ")\n" <<
    "└─\t" << ce_colors::fgwhite << "--profile" << RESET << ce_colors::fgwhite << "\t\t shows profiling information (how long each phase took)\n\n" <<

    // little command tip
    BOLD << ce_colors::fgwhite << "Tip" << RESET << ce_colors::fgwhite << ": run '" << ce_colors::bgblack << "cerne help <feature>" << RESET << ce_colors::fgwhite << "' for feature-specific information!\n\n" << RESET <<
    BOLD << ce_colors::fgblue << "Other Information" << RESET << ce_colors::fgwhite << ": https://docs.cerne.run \n" <<
    BOLD << ce_colors::fgwhite << "Copyright (c) 2026 Cerne Project" <<
    RESET << std::endl;
}