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

APP_C  ?= fanx

CONFIG_DIR ?= $(shell \
	dir="$(CURDIR)"; \
	while [ "$$dir" != "/" ]; do \
		if [ -d "$$dir/build-aux" ]; then \
			echo "$$dir/build-aux"; \
			break; \
		fi; \
		dir=$$(dirname "$$dir"); \
	done \
)

include $(CONFIG_DIR)/config.mk

# =============
# CONFIGURATION
# =============
# The entrypoint file
APP_SRC      := $(APP_DIR)/$(APP_C).cpp
CPP_SRC_FILES := $(APP_SRC)

# Manage building files
APP_EXE := $(APP_BUILD_DIR)/$(APP_C)
PKG_NAME ?= $(APP_EXE)

ALL_GEN_SRC += $(APP_EXE)

# All miscellaneous files generated
ALL_GEN_NM  :=  $(APP_EXE)_dbg $(APP_EXE)_dbg_asan $(APP_EXE)_dbg_ub $(APP_EXE)_gcov $(APP_EXE).debug
ALL_GEN_TAR := $(patsubst %, $(APP_BUILD_DIR)/%, *.tar.xz *.tar.gz *.tar.bz2)
ALL_GEN_MISC:= $(patsubst %, $(APP_BUILD_DIR)/%, core* vgcore* *.o *~ *.gcov *.gcda *.gcno *.ast)

# Shell scripts
SHELL_DIRS ?= $(SCRIPTS_DIR) $(CONFIG_DIR)
SHELL_FILES = $(shell find $(SHELL_DIRS) -name "*.sh" 2>/dev/null)

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

# --- Tools Configuration
CHECKPATCH  ?= $(KDIR)/scripts/checkpatch.pl
CLANG_FORMAT:= clang-format
FLAWFINDER  := flawfinder
CPPCHECK    := cppcheck
VALGRIND    := valgrind
GCOV        := gcov
TAR         := tar
CLANG_CHECK := clang-check
SHFMT      ?= shfmt
SHELLCHECK ?= shellcheck

# =====
# BUILD
# =====
.PHONY: all build prod prod_2part debug format format-check \
        sa sa_flawfinder sa_cppcheck print_ast valgrind covg \
        dist tarxz-pkg clean-dist dist-clean package clean help \
        env clear-env lint-shell format-shell mrproper

.DEFAULT_GOAL := help

all: clean format sa prod prod_2part debug

ifeq ($(DEBUG_MODE),1)
    BUILD_TARGET := debug
else
    BUILD_TARGET := prod_2part
endif

build: $(BUILD_TARGET)
prod: env $(APP_SRC)
	$(call print_step,Building 'production'-ready target ($(PROD_OPTLEVEL), no debug, stripped))
	$(call find_package,$(CXX))
	$(call find_package,$(STRIP))
	$(CXX) $(CXXFLAGS) $(APP_SRC) -o $(APP_EXE)
	$(STRIP) --strip-all $(APP_EXE)

prod_2part: debug
	$(call print_step,Building 'production'-ready 2-part target ($(PROD_OPTLEVEL), separate debug symbols))
# Check dependencies
	$(call find_package,$(CXX))
	$(call find_package,$(OBJCOPY))
	$(call find_package,$(STRIP))
	$(CXX) $(CXXFLAGS) $(APP_SRC) -o $(APP_EXE)
	$(OBJCOPY) --only-keep-debug $(APP_EXE) $(APP_EXE).debug
	$(STRIP) --strip-unneeded $(APP_EXE)
	$(OBJCOPY) --add-gnu-debuglink=$(APP_EXE).debug $(APP_EXE)

debug: env $(APP_SRC)
	$(call print_step,Building 'debug'-ready targets (with debug symbolic info, not stripped))
	$(call find_package,$(CXX))
	$(CXX) $(CXXFLAGS_DBG) $(APP_SRC) -o $(APP_EXE)_dbg
#-- Sanitizing
	$(CXX) $(CXXFLAGS_DBG_ASAN) $(APP_SRC) -o $(APP_EXE)_dbg_asan
	$(CXX) $(CXXFLAGS_DBG_UB) $(APP_SRC) -o $(APP_EXE)_dbg_ub
#	$(CXX) $(CXXFLAGS_DBG_MSAN) $(APP_SRC) -o $(APP_EXE)_dbg_msan

env:
	$(call print_step,Creating userspace application build environment)
# Will create all missing directories : mkdir -p ...
	$(BUILDER) $(BUILDER_FLAGS) --create --directory $(APP_BUILD_DIR)
	$(BUILDER) $(BUILDER_FLAGS) --copy --files $(APP_DIR)/Makefile  --directory $(APP_BUILD_DIR)
	$(BUILDER) $(BUILDER_FLAGS) --copy --files $(ROOT_DIR)/Makefile --directory $(BUILD_DIR)

clear-env:
	$(call print_step,Clearing userspace application build environment directories)
	rm -rf $(APP_BUILD_DIR)/*

# ==========
# CODE STYLE
# ==========
format: format-check format-clang format-shell

# clang-format reads the nearest .clang-format file (C++ style for this directory)
format-clang:
	$(call find_package,$(CLANG_FORMAT))
	$(call print_step,Format userspace source files with clang-format)
	find $(APP_DIR) $(ROOT_DIR)/include/uapi \
		-type f \( -name '*.c' -o -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \) \
		-exec $(CLANG_FORMAT) --fallback-style=none -i {} +

format-shell: lint-shell
	$(call find_package,$(SHFMT))
	$(call print_step,Formatting shell scripts with $(SHFMT))
	$(SHFMT) -w -i 4 -fn $(SHELL_FILES)

format-check: checkpatch clang-check

clang-check:
	$(call find_package,$(CLANG_FORMAT))
	$(call print_step,Check userspace formatting with clang-format (no file modified))
	find $(APP_DIR) $(ROOT_DIR)/include/uapi \
		-type f \( -name '*.c' -o -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \) \
		-exec $(CLANG_FORMAT) --fallback-style=none --dry-run --Werror {} +

checkpatch:
	$(call find_package,$(CHECKPATCH))
	$(call print_step,Running code style check with checkpatch.pl)
	$(CHECKPATCH) --no-tree -f --max-line-length=95 *.cpp *.hpp *.c *.h 2>/dev/null || true

lint-shell:
	$(call find_package,$(SHELLCHECK))
	$(call print_step,Linting shell scripts (POSIX sh) with $(SHELLCHECK))
	$(SHELLCHECK) -s sh -x $(SHELL_FILES)

# ====================
# SA - STATIC ANALYSIS
# ====================
sa: sa_flawfinder sa_cppcheck

sa_flawfinder:
	$(call find_package,$(FLAWFINDER))
	$(call print_step,Running static analysis with flawfinder)
	$(FLAWFINDER) --neverignore --context $(CPP_SRC_FILES)

sa_cppcheck:
	$(call find_package,$(CPPCHECK))
	$(call print_step,Running static analysis with cppcheck)
	$(CPPCHECK) -v --force --enable=all $(CXXSTD) --suppress=missingIncludeSystem .

print_ast:
	$(call find_package,$(CLANG_CHECK))
	$(call print_step,Static analysis with clang-check : ast-print (output: $(APP_EXE).ast))
	$(CLANG_CHECK) --ast-print $(APP_SRC) -- $(CXXSTD) > $(APP_EXE).ast 2>&1

# Dynamic analysis with valgrind
valgrind: debug
	$(call find_package,$(VALGRIND))
	$(call print_step,Dynamic analysis with valgrind)
	$(VALGRIND) --tool=memcheck --trace-children=yes $(APP_EXE)_dbg

# Testing: line coverage with gcov(1)
covg:
	$(call find_package,$(GCOV))
	$(call print_step,Code coverage with gcov)
	$(CXX) $(CXXFLAGS_GCOV) $(APP_SRC) -o $(APP_EXE)_gcov
	./$(APP_EXE)_gcov
	$(GCOV) $(APP_EXE)_gcov-$(APP_C).gcda
	$(call print_y,Examine the *.gcov file(s))

# ================
# ARCHIVE WITH TAR
# ================
dist: tarxz-pkg

package: tarxz-pkg

tarxz-pkg:
	$(call find_package,$(TAR))
	@rm -f $(PKG_NAME).tar.xz 2>/dev/null
	$(call print_step,Packaging $(PKG_NAME).tar.xz)
	$(TAR) caf $(PKG_NAME).tar.xz *
	@ls -lh $(PKG_NAME).tar.xz
	$(MAKE) clean
	$(call print_g,Package created: $(PKG_NAME).tar.xz)

clean-dist: dist-clean

dist-clean:
	$(call print_step,Removing archive files)
	rm -f $(PKG_NAME).tar.xz $(ALL_GEN_TAR) 2> /dev/null || true

clean:
	$(call print_step,Cleaning generated files)
	rm -f $(ALL_GEN_SRC) $(ALL_GEN_NM) $(ALL_GEN_MISC)

mrproper: clean
	$(call print_step,Removing entire app build directory)
	rm -rf $(APP_BUILD_DIR)

# ==============
# MAKE HELP MENU
# ==============
help:
	@echo
	$(call print_g,=== Makefile Help ===)
	$(call print_y,Usage: make <target> [APP_C=filename] [PKG_NAME=pkgname] [CROSS_COMPILE=prefix])
	@echo
	$(call print_g,Build Targets ::)
	@echo "  all            Run clean, format, static analysis, and build all targets"
	@echo "  build          Build regular production target (alias for 'prod')"
	@echo "  prod           Build 'production'-ready target ($(PROD_OPTLEVEL), no debug, stripped)"
	@echo "  prod_2part     Build production target with gnu_debuglink to debug symbols"
	@echo "  debug          Build debug targets (with debug info, ASan, and UBSan)"
	@echo "  env            Create the build environment directory structure"
	@echo "  clear-env      Clean the contents of the application build directory"
	@echo "  mrproper       Remove the entire build environment directory ($(APP_BUILD_DIR))"
	@echo "  clean          Remove binaries, object files and coverage artifacts"
	@echo "  help           Display this help message"
	@echo
	$(call print_g,Code Style Targets ::)
	@echo "  format         Format C/C++ sources in place with clang-format"
	@echo "  format-check   Verify formatting without modifying files (for CI)"
	@echo "  format-shell   Format shell scripts in place with shfmt"
	@echo
	$(call print_g,Static Analysis and Linting Targets ::)
	@echo "  sa             Run all static analysis tools (flawfinder, cppcheck)"
	@echo "  sa_flawfinder  Run security static analysis with flawfinder"
	@echo "  sa_cppcheck    Run static analysis with cppcheck"
	@echo "  print_ast      Generate Clang AST tree using clang-check"
	@echo "  lint-shell     Lint shell scripts (POSIX sh) with shellcheck"
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
	@echo "  APP_C          Source file name without .cpp extension (default: fanx)"
	@echo "  PKG_NAME       Name used for output package archive (default: \$$(APP_C))"
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