#	Cerne Compiler - File responsible for the Cerne Compiler's building.
#
#   Copyright (c) 2026 Cerne Project
#   SPDX-License-Identifier: LGPL-3.0-only
#
#   This file is part of the Cerne Compiler, licensed under LGPL-3.0.
#   See the LICENSE file in the root directory for details.
CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -g -O2 -I./src/include
SRC := ./src/
SRCS := $(shell find $(SRC_DIR) -name "*.cc")
TARGET := ./cerne

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean