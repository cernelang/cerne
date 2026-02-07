#	==================== Cerne::Compiler ====================
#
#       File responsible for the Cerne Compiler's building.
#
#   =========================================================
#
#   Last modified by Kashi | 07.02.26
#
#   Copyright (c) 2026 Cerne Project
#
#   This file is part of the Cerne Compiler, licensed under LGPL-3.0.
#   See the LICENSE file in the root directory for details.
CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -O2 -I./src/include
SRC := ./src/
SRCS := $(shell find $(SRC) -name "*.cc")
OBJS := $(SRCS:.cc=.o)
TARGET := ./cerne

all: $(TARGET) clean

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

.PHONY: all clean