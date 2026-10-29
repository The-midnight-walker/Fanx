# SPDX-License-Identifier: GPL-2.0
#
# *************************************************************
# This Makefile and its code patterns are inspired by the book:
#   "Linux Kernel Programming - Part 2"
#   Char Device Drivers and Kernel Synchronization
#   by Kaiwan N. Billimoria (Packt Publishing)
#   GitHub repository: https://github.com/PacktPublishing/Linux-Kernel-Programming-Part-2/blob/main/ch5/sed1/userapp_sed/Makefile
#
# An independent, rewritten, and extended adaptation.
# *************************************************************
#
# AUTHOR      : Midnight Walker
# DESCRIPTION : Userspace Application Makefile Template (C++)
# LICENSE     : GPLv2
# VERSION     : 0.5

APP ?= fanx

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
APP_SRC_DIR       := $(APP_DIR)/src
APP_BUILD_BIN_DIR := $(APP_BUILD_DIR)/bin
APP_BUILD_TAR_DIR := $(APP_BUILD_DIR)/dist
INCLUDE_DIRS      := $(ROOT_DIR)/include/uapi $(APP_DIR)/include $(APP_DIR)/include/lib
LIBS_DIRS         := $(APP_DIR)/lib
SHELL_DIRS        ?= $(SCRIPTS_DIR) $(CONFIG_DIR)

# The entrypoint file & sources
APP_SRC       := $(APP_SRC_DIR)/main.cpp
CPP_SRC_FILES := $(APP_SRC)
OBJS          := $(patsubst $(APP_SRC_DIR)/%.cpp, $(APP_BUILD_BIN_DIR)/%.o, $(CPP_SRC_FILES))
LIBS          := $(wildcard $(LIBS_DIRS)/*.so)

# Manage building files
APP_EXE       := $(APP_BUILD_BIN_DIR)/$(APP)
PKG_NAME      ?= $(APP_BUILD_TAR_DIR)/$(APP)

# Individual debug, coverage, and analysis outputs in $(APP_BUILD_DIR)
APP_DBG       := $(APP_BUILD_DIR)/$(APP)_dbg
APP_DBG_ASAN  := $(APP_BUILD_DIR)/$(APP)_dbg_asan
APP_DBG_UBSAN    := $(APP_BUILD_DIR)/$(APP)_dbg_ubsan
#APP_DBG_MSAN := $(APP_BUILD_DIR)/$(APP)_dbg_msan
APP_GCOV      := $(APP_BUILD_DIR)/$(APP)_gcov
APP_DEBUG_SYM := $(APP_BUILD_DIR)/$(APP).debug
APP_AST       := $(APP_BUILD_DIR)/$(APP).ast

# Artifact collections (original names strictly preserved)
ALL_GEN_SRC   := $(OBJS) $(APP_EXE)
ALL_GEN_NM    := $(APP_DBG) $(APP_DBG_ASAN) $(APP_DBG_UBSAN) $(APP_GCOV) $(APP_DEBUG_SYM) #$(APP_DBG_MSAN)

# File patterns for distribution archives and miscellaneous cleanup
TAR_PATTERNS  := *.tar.xz *.tar.gz *.tar.bz2
MISC_PATTERNS := core* vgcore* *~ *.gcov *.gcda *.gcno

ALL_GEN_TAR   := $(PKG_NAME).tar.xz $(addprefix $(APP_BUILD_TAR_DIR)/, $(TAR_PATTERNS))
ALL_GEN_MISC  := $(addprefix $(APP_BUILD_DIR)/, $(MISC_PATTERNS)) $(APP_AST) $(APP_BUILD_BIN_DIR)/*.d

# Shell scripts
SHELL_FILES   := $(shell find $(SHELL_DIRS) -name "*.sh" 2>/dev/null)

# --- Toolchain
CXX     := $(CROSS_COMPILE)g++
CL      := $(CROSS_COMPILE)clang
STRIP   := $(CROSS_COMPILE)strip
OBJCOPY := $(CROSS_COMPILE)objcopy

CXXSTD        := -std=c++17
PROD_OPTLEVEL := -O2 # -O3 or -Os

# --- Compiler Flags
INCLUDES          := $(patsubst %, -I%, $(INCLUDE_DIRS))
CXXFLAGS          := $(CXXSTD) -Wall -UDEBUG $(PROD_OPTLEVEL) $(INCLUDES) -MMD -MP
CXXFLAGS_DBG      := $(CXXSTD) -g -ggdb -gdwarf-4 -O0 -Wall -Wextra -DDEBUG $(INCLUDES)
CXXFLAGS_DBG_ASAN := $(CXXFLAGS_DBG) -fsanitize=address
CXXFLAGS_DBG_UBSAN   := $(CXXFLAGS_DBG) -fsanitize=undefined
#CXXFLAGS_DBG_MSAN:= $(CXXFLAGS_DBG) -fsanitize=memory
CXXFLAGS_GCOV     := $(CXXFLAGS_DBG) -fprofile-arcs -ftest-coverage

# --- Tools Configuration
CHECKPATCH   ?= $(if $(wildcard $(KDIR)/scripts/checkpatch.pl),$(KDIR)/scripts/checkpatch.pl,checkpatch.pl)
CLANG_FORMAT := clang-format
FLAWFINDER   := flawfinder
CPPCHECK     := cppcheck
VALGRIND     := valgrind
GCOV         := gcov
TAR          := tar
CLANG_CHECK  := clang-check
SHFMT        ?= shfmt
SHELLCHECK   ?= shellcheck

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

prod: env $(OBJS)
	$(call print_step,Building 'production'-ready target ($(PROD_OPTLEVEL), no debug, stripped))
	$(call find_package,$(CXX))
	$(call find_package,$(STRIP))
	$(Q)$(CXX) $(OBJS) $(LIBS) -o $(APP_EXE)
	$(Q)$(STRIP) --strip-all $(APP_EXE)

prod_2part: env debug $(OBJS)
	$(call print_step,Building 'production'-ready 2-part target ($(PROD_OPTLEVEL), separate debug symbols))
	$(call find_package,$(CXX))
	$(call find_package,$(OBJCOPY))
	$(call find_package,$(STRIP))
	$(Q)$(CXX) $(OBJS) $(LIBS) -o $(APP_EXE)
	$(Q)$(OBJCOPY) --only-keep-debug $(APP_EXE) $(APP_DEBUG_SYM)
	$(Q)$(STRIP) --strip-unneeded $(APP_EXE)
	$(Q)$(OBJCOPY) --add-gnu-debuglink=$(APP_DEBUG_SYM) $(APP_EXE)

debug: env $(CPP_SRC_FILES)
	$(call print_step,Building 'debug'-ready targets (with debug symbolic info, not stripped))
	$(call find_package,$(CXX))
	$(Q)$(CXX) $(CXXFLAGS_DBG) $(CPP_SRC_FILES) $(LIBS) -o $(APP_DBG)
	$(Q)$(CXX) $(CXXFLAGS_DBG_ASAN) $(CPP_SRC_FILES) $(LIBS) -o $(APP_DBG_ASAN)
	$(Q)$(CXX) $(CXXFLAGS_DBG_UBSAN) $(CPP_SRC_FILES) $(LIBS) -o $(APP_DBG_UBSAN)
#	$(Q)$(CXX) $(CXXFLAGS_DBG_MSAN) $(CPP_SRC_FILES) $(LIBS) -o $(APP_DBG_MSAN)

$(APP_BUILD_BIN_DIR)/%.o: $(APP_SRC_DIR)/%.cpp | env
	$(Q)$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(OBJS:.o=.d)

env:
	$(call print_step,Creating userspace application build environment)
	$(Q)$(BUILDER) $(BUILDER_FLAGS) --create --directory $(APP_BUILD_BIN_DIR)
	$(Q)$(BUILDER) $(BUILDER_FLAGS) --create --directory $(APP_BUILD_TAR_DIR)
	$(Q)$(BUILDER) $(BUILDER_FLAGS) --copy --files $(APP_DIR)/Makefile  --directory $(APP_BUILD_DIR)
	$(Q)$(BUILDER) $(BUILDER_FLAGS) --copy --files $(ROOT_DIR)/Makefile --directory $(BUILD_DIR)

clear-env:
	$(call print_step,Clearing userspace application build environment directories)
	$(Q)rm -rf $(APP_BUILD_DIR)/*

# ==========
# CODE STYLE
# ==========
format: format-clang format-shell

format-clang:
	$(call find_package,$(CLANG_FORMAT))
	$(call print_step,Formatting userspace source files with clang-format)
	$(Q)find $(APP_SRC_DIR) $(INCLUDE_DIRS) \
		-type f \( -name '*.c' -o -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \) \
		-exec $(CLANG_FORMAT) --fallback-style=none -i {} +

format-shell: lint-shell
	$(call find_package,$(SHFMT))
	$(call print_step,Formatting shell scripts with $(SHFMT))
	$(Q)$(SHFMT) -w -i 4 -fn $(SHELL_FILES)

format-check: checkpatch clang-check

clang-check:
	$(call find_package,$(CLANG_FORMAT))
	$(call print_step,Checking userspace formatting with clang-format (dry-run))
	$(Q)find $(APP_SRC_DIR) $(INCLUDE_DIRS) \
		-type f \( -name '*.c' -o -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \) \
		-exec $(CLANG_FORMAT) --fallback-style=none --dry-run --Werror {} +

checkpatch:
	@if command -v $(CHECKPATCH) >/dev/null 2>&1 || [ -x "$(CHECKPATCH)" ]; then \
		$(BUILDER) $(BUILDER_FLAGS) --print blue "==> Running code style check with checkpatch.pl"; \
		$(CHECKPATCH) --no-tree -f --max-line-length=95 $(APP_SRC_DIR)/*.cpp $(APP_DIR)/include/*.h 2>/dev/null || true; \
	else \
		$(BUILDER) $(BUILDER_FLAGS) --print yellow "checkpatch.pl not found, skipping kernel style check"; \
	fi

lint-shell:
	$(call find_package,$(SHELLCHECK))
	$(call print_step,Linting shell scripts (POSIX sh) with $(SHELLCHECK))
	$(Q)$(SHELLCHECK) -s sh -x $(SHELL_FILES)

# ====================
# SA - STATIC ANALYSIS
# ====================
sa: sa_flawfinder sa_cppcheck

sa_flawfinder:
	$(call find_package,$(FLAWFINDER))
	$(call print_step,Running security static analysis with flawfinder)
	$(Q)$(FLAWFINDER) --neverignore --context $(CPP_SRC_FILES)

sa_cppcheck:
	$(call find_package,$(CPPCHECK))
	$(call print_step,Running static analysis with cppcheck)
	$(Q)$(CPPCHECK) -v --force --enable=all $(subst -std=,--std=,$(CXXSTD)) $(INCLUDES) --suppress=missingIncludeSystem $(APP_SRC_DIR)

print_ast: env
	$(call find_package,$(CLANG_CHECK))
	$(call print_step,Generating Clang AST tree (output: $(APP_AST)))
	$(Q)$(CLANG_CHECK) --ast-print $(CPP_SRC_FILES) -- $(CXXSTD) $(INCLUDES) > $(APP_AST) 2>&1

# ================
# DYNAMIC ANALYSIS
# ================
valgrind: debug
	$(call find_package,$(VALGRIND))
	$(call print_step,Running dynamic analysis with Valgrind)
	$(Q)$(VALGRIND) --tool=memcheck --trace-children=yes $(APP_DBG)

covg: env
	$(call print_step,Running code coverage analysis with gcov)
	$(Q)$(CXX) $(CXXFLAGS_GCOV) $(CPP_SRC_FILES) $(LIBS) -o $(APP_GCOV)
	$(Q)cd $(APP_BUILD_DIR) && ./$(notdir $(APP_GCOV))
	$(Q)cd $(APP_BUILD_DIR) && $(GCOV) $(notdir $(APP_GCOV))-*.gcda 2>/dev/null || true
	$(call print_y,Examine the generated *.gcov file(s))

# ================
# ARCHIVE WITH TAR
# ================
dist: tarxz-pkg

package: tarxz-pkg

tarxz-pkg: clean env
	$(call find_package,$(TAR))
	$(Q)rm -f $(PKG_NAME).tar.xz 2>/dev/null
	$(call print_step,Packaging $(PKG_NAME).tar.xz)
	$(Q)$(TAR) --exclude='.git' --exclude='build' -caf $(PKG_NAME).tar.xz -C $(APP_DIR) .
	$(Q)ls -lh $(PKG_NAME).tar.xz
	$(call print_g,Package created: $(PKG_NAME).tar.xz)

clean-dist: dist-clean

dist-clean:
	$(call print_step,Removing archive files)
	$(Q)rm -f $(ALL_GEN_TAR) 2>/dev/null || true

clean:
	$(call print_step,Cleaning generated files)
	$(Q)rm -f $(ALL_GEN_SRC) $(ALL_GEN_NM) $(ALL_GEN_MISC) 2>/dev/null || true

mrproper: clean dist-clean
	$(call print_step,Removing entire app build directory)
	$(Q)rm -rf $(APP_BUILD_DIR)

# ==============
# MAKE HELP MENU
# ==============
help:
	@echo
	$(call print_g,=== Application Makefile Help ===)
	$(call print_y,Usage: make <target> [APP=filename] [PKG_NAME=pkgname] [CROSS_COMPILE=prefix])
	@echo
	$(call print_g,Build Targets ::)
	@echo "  all            Run clean, format, static analysis, and build all targets"
	@echo "  build          Build regular production target (alias for 'prod')"
	@echo "  prod           Build 'production'-ready target ($(PROD_OPTLEVEL), stripped)"
	@echo "  prod_2part     Build production target with gnu_debuglink to separated debug symbols"
	@echo "  debug          Build debug targets (debug info, ASan, and UBSan)"
	@echo "  env            Create the build environment directory structure"
	@echo "  clear-env      Clean the contents of the application build directory"
	@echo "  clean          Remove binaries, object files, and coverage artifacts"
	@echo "  dist-clean     Remove generated tar archives (alias: clean-dist)"
	@echo "  mrproper       Completely remove the application build directory ($(APP_BUILD_DIR))"
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
	@echo "  dist           Create source tarball archive (alias: tarxz-pkg)"
	@echo "  package        Create source tarball archive (alias: tarxz-pkg)"
	@echo "  tarxz-pkg      Create a .tar.xz source archive in $(APP_BUILD_TAR_DIR)/"
	@echo "  dist-clean     Remove generated tar archives (alias: clean-dist)"
	@echo "  clean-dist     Remove generated tar archives"
	@echo
	$(call print_g,Variables ::)
	@echo "  V               Kernel-style verbose flag (1 = show commands, default: 0)"
	@echo "  C               Enable/disable colors (default: 1)"
	@echo "  B               Enable/disable bold text (default: 0)"
	@echo "  APP            Target binary name (default: fanx)"
	@echo "  PKG_NAME       Name prefix used for output archive (default: \$$(APP))"
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
