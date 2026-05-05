#	Cerne Compiler - File responsible for the Cerne Compiler's building.
#
#   Copyright (c) 2026 Cerne Project
#   SPDX-License-Identifier: LGPL-3.0-only
#
#   This file is part of the Cerne Compiler, licensed under LGPL-3.0.
#   See the LICENSE file in the root directory for details.

cmake_minimum_required(VERSION 3.15)
project(cerne LANGUAGES CXX)

# C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# source files
file(GLOB_RECURSE SOURCES "src/*.cc")

# executable
add_executable(cerne ${SOURCES})

# include directories
target_include_directories(cerne PRIVATE "src/include")

# during development, sanitizers are useful, but when releasing valgrind is a bit better, and since valgrind can't accept a binary with sanitizers on, this should be an option
option(USE_SANITIZERS "Use sanitizers during development" OFF)

if(USE_SANITIZERS)
    if(MSVC)
        message(WARNING "Sanitizers are not supported on MSVC, ignoring USE_SANITIZERS option.")
    else()
        target_compile_options(cerne PRIVATE -fsanitize=address,undefined)
        target_link_libraries(cerne PRIVATE -fsanitize=address,undefined)
    endif()
endif()

# now the warnings options (we want to enable all warnings)
if(MSVC)
    target_compile_options(cerne PRIVATE /W4)
else()
    target_compile_options(cerne PRIVATE -Wall -Wextra)
endif()