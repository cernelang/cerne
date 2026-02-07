/*
    Cerne Compiler - Main utility for the CLI interface.

    Copyright (c) 2026 Cerne Project
    SPDX-License-Identifier: LGPL-3.0-only

    This file is part of the Cerne Compiler.
    See the LICENSE file in the root directory for further details.
*/
#include "../include/utils.hpp"

const std::vector<std::string> commands = {"help", "version"};

cerne::args cerne::parse_args(int argc, char** argv) {
    cerne::args args;
    std::string current_arg = "";
    const bool def_arg_value = true;
    
    // for files
    size_t amt_files = 0;
    size_t init_size = 8;

    for(int i = 1; i < argc; i++) {
        std::string arg(argv[i]);

        if(arg[0] == '-') {
            // if there was a current argument and no value was assigned to it, assign it a default value (true) before creating a new argument
            if(current_arg.size() > 0) {
                args[current_arg] = (void*)&def_arg_value;
                current_arg = "";
            }

            int to_remove = 1;
            char second_char = arg[1];
            if(second_char == '-') to_remove = 2; 

            // --name=Something
            const std::size_t& pos_equ = arg.find("=");
            if(pos_equ != arg.npos) {
                std::string name = arg.substr(to_remove, pos_equ - to_remove);
                std::string value = arg.substr(pos_equ + 1);
                
                char* value_copy = (char*)malloc(value.length() + 1);
                strcpy(value_copy, value.c_str());
                args[name] = (void*)value_copy;
                continue;
            }

            // -n Something
            std::string name = arg.substr(to_remove);
            current_arg = name;
        } else {
            if(current_arg.size() > 0) {
                char* arg_copy = (char*)malloc(arg.length() + 1);
                strcpy(arg_copy, arg.c_str());
                args[current_arg] = (void*)arg_copy;
                current_arg = "";
            } 
            // for raw arguments, we first check whether they are commands or not and if they're NOT in the list of commands, they will be considered files instead
            else {
                auto it = std::find(commands.begin(), commands.end(), arg);

                // if it's a command
                if(it != commands.end()) {
                    args[arg] = (void*)&def_arg_value;
                }
                // else it's considered a file
                else {
                    char** files = (char**)args["files"];
                    
                    // is this the first file?
                    if(files == nullptr) files = (char**)malloc(init_size * sizeof(char*));
                    else {
                        if(amt_files+1 >= init_size) {
                            init_size*=2;
                            files = (char**)realloc(files, init_size * sizeof(char*));
                        }
                    }
                    
                    files[amt_files] = (char*)malloc(arg.length() + 1);
                    strcpy(files[amt_files], arg.c_str());

                    amt_files++;
                    args["files"] = files;
                }
            }
        }
    }

    // for any flags at the end like cerne <whatever> --flag
    //                                                ^ 
    // this flag won't log thanks to the loop's logic, so outside of it check if there is any flag and push it as "true"
    if(current_arg.size() > 0) args[current_arg] = (void*)&def_arg_value;

    int* size_ptr = (int*)malloc(sizeof(int));
    *size_ptr = amt_files;
    args["files_size"] = size_ptr;

    // no arguments
    if(argc == 1) args["help"] = (void*)&def_arg_value;

    return args;
}

cerne::CLI::CLI(const cerne::args& args) {
    this->__args = args;
}

cerne::CLI::~CLI() {}

void cerne::CLI::event(std::string type, cerne::callback fnc) {
    if(this->__args.find(type) != this->__args.end()) {
        int temp = 0;
        if(type == "files") temp = *(int*)this->__args["files_size"];

        char** args = static_cast<char**>(this->__args[type]);

        std::visit([args, temp](auto&& arg) {
            using _type = std::decay_t<decltype(arg)>;

            if constexpr (std::is_invocable_v<_type, char**, int>) {
                arg(args, temp);
            } else {
                arg();
            }
        }, fnc);
    }
}