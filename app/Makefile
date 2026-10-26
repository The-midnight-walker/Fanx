# SPDX-License-Identifier: GPL-2.0
#
# *************************************************************
# This Makefile and its code patterns are inspired by the book:
#   "Linux Kernel Programming - Part 2"
#   Char Device Drivers and Kernel Synchronization
#   by Kaiwan N. Billimoria (Packt Publishing)
#   github repository : https://github.com/PacktPublishing/Linux-Kernel-Programming-Part-2/blob/main/ch5/sed1/userapp_sed/Makefile
#
# An independent, rewritten, and extended adaptation.
#
# *************************************************************
#
# AUTHOR      : Midnight Walker
# DESCRIPTION : Userspace Application Makefile Template (C++)
# LICENSE     : GPLv2
# VERSION     : 0.4

ROOT_DIR?=..

# =============
# CONFIGURATION
# =============
APK_C    ?= fanx
PKG_NAME ?= $(APK_C)
SRC      := $(APK_C).cpp

ALL      := prod prod_2part debug
ALL_NM   := $(APK_C) $(APK_C)_dbg $(APK_C)_dbg_asan $(APK_C)_dbg_ub $(APK_C)_gcov

# --- Toolchain
CXX      := $(CROSS_COMPILE)g++
CL       := $(CROSS_COMPILE)clang
STRIP    := $(CROSS_COMPILE)strip
OBJCOPY  := $(CROSS_COMPILE)objcopy

CXXSTD        := -std=c++17
PROD_OPTLEVEL := -O2 # -O3 or -Os

# --- Compiler Flags
CXXFLAGS          := $(CXXSTD) -Wall -UDEBUG $(PROD_OPTLEVEL)
CXXFLAGS_DBG      := $(CXXSTD) -g -ggdb -gdwarf-4 -O0 -Wall -Wextra -DDEBUG
CXXFLAGS_DBG_ASAN := $(CXXFLAGS_DBG) -fsanitize=address
CXXFLAGS_DBG_UB   := $(CXXFLAGS_DBG) -fsanitize=undefined
#CXXFLAGS_DBG_MSAN:= $(CXXFLAGS_DBG) -fsanitize=memory
CXXFLAGS_GCOV     := $(CXXFLAGS_DBG) -fprofile-arcs -ftest-coverage

# --- Tool Configuration
SRC_FILES   := $(wildcard *.c *.h *.cpp *.hpp)

CHECKPATCH  ?= $(KDIR)/scripts/checkpatch.pl
CLANG_FORMAT:= clang-format
FLAWFINDER  := flawfinder
CPPCHECK    := cppcheck
VALGRIND    := valgrind
GCOV        := gcov
TAR         := tar
CLANG_CHECK := clang-check

# Outputs
COLOR   ?= 1
#IN_BOLD ?= 0

include $(ROOT_DIR)/build-aux/utils.mk

# =====
# BUILD
# =====
.PHONY: all build prod prod_2part debug format format-check \
        sa sa_flawfinder sa_cppcheck print_ast valgrind covg \
        dist tarxz-pkg clean-dist dist-clean package clean help

.DEFAULT_GOAL := help

all: clean format sa prod prod_2part debug

build: prod

prod: $(SRC)
	$(call print_step,Building 'production'-ready target ($(PROD_OPTLEVEL), no debug, stripped))
	$(CXX) $(CXXFLAGS) $(SRC) -o $(APK_C)
	$(STRIP) --strip-all ./$(APK_C)

prod_2part: debug $(SRC)
	$(call print_step,Building 'production'-ready 2-part target ($(PROD_OPTLEVEL), no debug, stripped))
	$(CXX) $(CXXFLAGS) $(SRC) -o $(APK_C)
	$(STRIP) --strip-debug ./$(APK_C)
	$(OBJCOPY) --add-gnu-debuglink=./$(APK_C)_dbg ./$(APK_C)

debug: $(SRC)
	$(call print_step,Building 'debug'-ready targets (with debug symbolic info, not stripped))
	$(CXX) $(CXXFLAGS_DBG) $(SRC) -o $(APK_C)_dbg
#-- Sanitizing
	$(CXX) $(CXXFLAGS_DBG_ASAN) $(SRC) -o $(APK_C)_dbg_asan
	$(CXX) $(CXXFLAGS_DBG_UB) $(SRC) -o $(APK_C)_dbg_ub
#	$(CXX) $(CXXFLAGS_DBG_MSAN) $(SRC) -o $(APK_C)_dbg_msan

# ==========
# CODE STYLE
format: format-check format-clang
# ==========
# clang-format reads the nearest .clang-format file (C++ style for this directory)
format-clang:
ifeq (,$(shell command -v $(CLANG_FORMAT) 2>/dev/null))
	$(call print_error,'$(CLANG_FORMAT)' tool is not installed on your host)
endif
	$(call print_step,Format userspace source files with clang-format)
	find . \( -type d \( -name .git -o -name build -o -name 'cmake-build-*' \) -prune \) -o \
		-type f \( -name '*.c' -o -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \) \
		-exec $(CLANG_FORMAT) --fallback-style=none -i {} +

format-check: checkpatch clang-check

clang-check:
ifeq (,$(shell command -v $(CLANG_FORMAT) 2>/dev/null))
	$(call print_error,'$(CLANG_FORMAT)' tool is not installed on your host)
endif
	$(call print_step,Check userspace formatting with clang-format (no file modified))
	find . \( -type d \( -name .git -o -name build -o -name 'cmake-build-*' \) -prune \) -o \
		-type f \( -name '*.c' -o -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \) \
		-exec $(CLANG_FORMAT) --fallback-style=none --dry-run --Werror {} +

checkpatch:
ifeq (,$(shell command -v $(CHECKPATCH) 2>/dev/null))
	$(call print_step,Running code style check with checkpatch.pl)
	$(CHECKPATCH) --no-tree -f --max-line-length=95 *.cpp *.hpp *.c *.h 2>/dev/null || true
else
	$(call print_err,Checkpatch.pl script not found at $(CHECKPATCH).)
endif

# ====================
# SA - STATIC ANALYSIS
# ====================
sa: sa_flawfinder sa_cppcheck

sa_flawfinder:
ifeq (,$(shell command -v $(FLAWFINDER) 2>/dev/null))
	$(call print_error,$(FLAWFINDER) is not installed on your host)
endif
	$(call print_step,Running static analysis with flawfinder)
	$(FLAWFINDER) --neverignore --context $(SRC_FILES)

sa_cppcheck:
ifeq (,$(shell command -v $(CPPCHECK) 2>/dev/null))
	$(call print_error,$(CPPCHECK) is not installed on your host)
endif
	$(call print_step,Running static analysis with cppcheck)
	$(CPPCHECK) -v --force --enable=all -$(CXXSTD) --suppress=missingIncludeSystem .

print_ast:
ifeq (,$(shell command -v $(CLANG_CHECK) 2>/dev/null))
	$(call print_error,'$(CLANG_CHECK)' tool is not installed on your host)
endif
	$(call print_step,Static analysis with clang-check : ast-print (output: $(SRC).ast))
	$(CLANG_CHECK) --ast-print $(SRC) -- $(CXXSTD) > $(SRC).ast 2>&1

# Dynamic analysis with valgrind
valgrind: debug
ifeq (,$(shell command -v $(VALGRIND) 2>/dev/null))
	$(call print_error,'$(VALGRIND)' tool is not installed on your host)
endif
	$(call print_step,Dynamic analysis with valgrind)
	$(VALGRIND) --tool=memcheck --trace-children=yes ./$(APK_C)_dbg

# Testing: line coverage with gcov(1)
covg:
ifeq (,$(shell command -v $(GCOV) 2>/dev/null))
	$(call print_error,'$(GCOV)' tool is not installed on your host)
endif
	$(call print_step,Code coverage with gcov)
	$(CXX) $(CXXFLAGS_GCOV) $(SRC) -o $(APK_C)_gcov
	./$(APK_C)_gcov
	$(GCOV) $(APK_C)_gcov-$(APK_C).gcda
	@echo "Examine the *.gcov file(s)"

# ================
# ARCHIVE WITH TAR
# ================
dist: tarxz-pkg

package: tarxz-pkg

tarxz-pkg: clean
ifeq (,$(shell command -v $(TAR) 2>/dev/null))
	$(call print_error,'$(TAR)' tool is not installed on your host)
endif
	@rm -f ../$(PKG_NAME).tar.xz 2>/dev/null
	$(call print_step,Packaging $(PKG_NAME).tar.xz)
	$(TAR) caf ../$(PKG_NAME).tar.xz *
	@ls -lh ../$(PKG_NAME).tar.xz
	$(call print_g,Package created: ../$(PKG_NAME).tar.xz)

clean-dist: dist-clean

dist-clean:
	$(call print_err,Deleting archive files...)
	rm -f ../$(PKG_NAME).tar.xz *.tar.gz *.tar.xz *.tar.bz2 2>/dev/null || true

clean:
	$(call print_step,Cleaning generated files)
	rm -rf $(ALL_NM) core* vgcore* *.o *~ *.gcov *.gcda *.gcno *.ast

# ==============
# MAKE HELP MENU
# ==============
help:
	@echo
	$(call print_g,=== Makefile Help ===)
	$(call print_y,Usage: make <target> [APK_C=filename] [PKG_NAME=pkgname] [CROSS_COMPILE=prefix])
	@echo
	$(call print_g,Build Targets ::)
	@echo "  all            Run clean, format, static analysis, and build all targets"
	@echo "  build          Build regular production target (alias for 'prod')"
	@echo "  prod           Build 'production'-ready target ($(PROD_OPTLEVEL), no debug, stripped)"
	@echo "  prod_2part     Build production target with gnu_debuglink to debug symbols"
	@echo "  debug          Build debug targets (with debug info, ASan, and UBSan)"
	@echo "  clean          Remove binaries, object files and coverage artifacts"
	@echo "  help           Display this help message"
	@echo
	$(call print_g,Code Style Targets ::)
	@echo "  format         Format C/C++ sources in place with clang-format"
	@echo "  format-check   Verify formatting without modifying files (for CI)"
	@echo
	$(call print_g,Static Analysis Targets ::)
	@echo "  sa             Run all static analysis tools (flawfinder, cppcheck)"
	@echo "  sa_flawfinder  Run security static analysis with flawfinder"
	@echo "  sa_cppcheck    Run static analysis with cppcheck"
	@echo "  print_ast      Generate Clang AST tree using clang-check"
	@echo
	$(call print_g,Dynamic Analysis Targets ::)
	@echo "  valgrind       Run memory check on debug binary with valgrind"
	@echo "  covg           Run code coverage analysis using gcov"
	@echo
	$(call print_g,Packaging & Distribution Targets ::)
	@echo "  dist           Alias for tarxz-pkg"
	@echo "  package        Alias for tarxz-pkg"
	@echo "  tarxz-pkg      Create a .tar.xz source archive in parent directory"
	@echo "  dist-clean     Remove generated tar archives (alias: clean-dist)"
	@echo "  clean-dist     Remove generated tar archives"
	@echo
	$(call print_g,Variables ::)
	@echo "  APK_C          Source file name without .cpp extension (default: fanx)"
	@echo "  PKG_NAME       Name used for output package archive (default: \$$(APK_C))"
	@echo "  CROSS_COMPILE  Cross-compiler toolchain prefix (e.g., arm-linux-gnueabihf-)"
	@echo "  CXX            C++ compiler binary (default: \$$(CROSS_COMPILE)g++)"
	@echo "  CXXSTD         C++ standard flag (default: -std=c++17)"
	@echo "  PROD_OPTLEVEL  Optimization level for prod target (default: -O2)"
	@echo "  CLANG_FORMAT   Formatter binary (default: clang-format)"
	@echo "  FLAWFINDER     Flawfinder binary (default: flawfinder)"
	@echo "  CPPCHECK       Cppcheck binary (default: cppcheck)"
	@echo "  CLANG_CHECK    Clang AST analyzer binary (default: clang-check)"
	@echo "  VALGRIND       Valgrind binary (default: valgrind)"
	@echo "  GCOV           Gcov coverage tool binary (default: gcov)"
	@echo "  TAR            Archiving tool binary (default: tar)"
	@echo