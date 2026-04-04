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
        const std::string_view& name = arg.substr(rem, pos_equ-rem),
                                value = arg.substr(pos_equ + 1);

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
            std::string_view next_arg = std::string_view(next);
            
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
            if(std::find(commands.begin(), commands.end(), arg) != commands.end()) {
                auto _flag = std::pair<std::string, std::string>(arg, "");
                args.flags.insert(_flag);
                continue;
            }
            
            args.files.push_back(arg);
        }
    }

    // no flags or files? then print the help function
    if(args.flags.size() == 0 && args.files.size() == 0) args.flags.insert({ "help", "" });

    return args;
}

/**
 * Handles the events for the CLI
 * taking the type of event (could be a command or "files" to get the files) and calling the appropriate callback function
 */
void cerne::CLI::event(std::string type, cerne::callback fnc) {
    const auto& args = this->__args;
    const auto& flags = args.flags;
    const auto& files = args.files;

    std::visit([&flags, &files, &type](auto&& new_fnc) {
        using fnc_stripped = std::decay_t<decltype(new_fnc)>;

        if constexpr (std::is_invocable_v<fnc_stripped, std::vector<std::string>>) {
            new_fnc(files);
        } else {
            if(flags.find(type) != flags.end()) new_fnc();
        }
    }, fnc);
}

/**
 * Provides a helpful message for the users who are unsure of how to use the CLI or any of the tools
 * or just want to know more about the compiler in general.
 */
void cerne::CLI::help() const {
    std::string logo = R"raw(
 ██████╗███████╗██████╗ ███╗   ██╗███████╗
██╔════╝██╔════╝██╔══██╗████╗  ██║██╔════╝
██║     █████╗  ██████╔╝██╔██╗ ██║█████╗  
██║     ██╔══╝  ██╔══██╗██║╚██╗██║██╔══╝  
╚██████╗███████╗██║  ██║██║ ╚████║███████╗
 ╚═════╝╚══════╝╚═╝  ╚═╝╚═╝  ╚═══╝╚══════╝
)raw";

    std::cout << FG "255m" << logo << 

    "\n\n" <<
    
    "Welcome to Cerne!\n\n" << 
    // command usage just in case the user doesn't know how to use the CLI or just wants a reminder
    
    FG "218;4;1m" << "Command Usage\n" << ESC "[0m" 
    FG "223;24m" << "> " << FG "189m" << "cerne " << FG "255m" << "<command> [flags] [files]\n\n" <<
    
    // command list in the help message
    FG "121;4;1m" << "Available Commands\n" << ESC "[0m" 
    FG "223;24m" << "> " << FG "195m" << "help" << FG "255m" << " - shows this message\n" <<
    FG "223;24m" << "> " << FG "195m" << "version" << FG "255m" << " - shows the current version of the compiler\n\n" <<
    
    // signature from me (kashi [quick side note - this is an alias, not my real name]) and a link to the website
    FG "153;1m" << "Created by Kashi" << FG "237m" << " | " << FG "122;1m" << "https://cerne.space" <<
    RESET << std::endl;
}