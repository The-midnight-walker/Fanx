# SPDX-License-Identifier: GPL-2.0
#
# *************************************************************
# This Makefile and its code patterns are inspired by the book:
#   "Linux Kernel Programming - Part 2"
#   Char Device Drivers and Kernel Synchronization
#   by Kaiwan N. Billimoria (Packt Publishing)
#
# An independent, rewritten, and extended adaptation.
# *************************************************************
#
# AUTHOR      : Midnight Walker
# DESCRIPTION : Kernel module Makefile Template
# LICENSE     : GPLv2
# VERSION     : 0.1

# IMPORTANT : Set MOD_NAME_C to the kernel module name
# source filename without the '.c' file extension.
MOD_NAME_C ?= fanx

ifeq ($(KERNELRELEASE),)
CONFIG_DIR := $(shell \
	dir="$(CURDIR)"; \
	while [ "$$dir" != "/" ]; do \
		if [ -d "$$dir/build-aux" ]; then \
			echo "$$dir/build-aux"; \
			break; \
		fi; \
		dir=$$(dirname "$$dir"); \
	done \
)
ifeq ($(strip $(CONFIG_DIR)),)
$(error Could not find build-aux/config.mk from $(CURDIR))
endif
endif

# Resolve to the source directory even when this copied Makefile is invoked
# from build/kernel. During kbuild's recursive read, keep its staged location.
ifeq ($(KERNELRELEASE),)
KERNEL_SRC_DIR ?= $(abspath $(CONFIG_DIR)/../kernel)
else
KERNEL_SRC_DIR ?= $(abspath $(dir $(firstword $(MAKEFILE_LIST))))
endif

# Executed only when called directly by the user (KERNELRELEASE is unset).
# Prevents kbuild from entering an infinite recursion loop when re-evaluating.
ifeq ($(KERNELRELEASE),)
    include $(CONFIG_DIR)/config.mk
endif

# =============
# CONFIGURATION
# =============
# Specify other obj (.o) files for multi-file modules
$(MOD_NAME_C)-objs ?=

# Keep module build artifacts in the shared build/kernel directory regardless
# of whether make is invoked from kernel/ or build/.
KERNEL_BUILD_DIR ?= $(if $(BUILD_DIR),$(abspath $(if $(DRV_BUILD_DIR),$(DRV_BUILD_DIR),$(BUILD_DIR)/kernel)),$(abspath $(dir $(firstword $(MAKEFILE_LIST)))))
KERNEL_DIST_DIR  ?= $(KERNEL_BUILD_DIR)/dist
KERNEL_ENV_STAMP := $(KERNEL_BUILD_DIR)/.env.stamp
KERNEL_ENV_FILES := $(ROOT_DIR)/Makefile $(KERNEL_SRC_DIR)/Makefile $(wildcard $(KERNEL_SRC_DIR)/Kbuild $(KERNEL_SRC_DIR)/Kconfig)
SHELL_DIRS   ?= $(SCRIPTS_DIR) $(CONFIG_DIR)
INCLUDE_DIRS ?= $(ROOT_DIR)/include $(ROOT_DIR)/include/uapi
KERNEL_INCLUDE_DIRS := $(sort $(abspath $(ROOT_DIR) $(INCLUDE_DIRS)))
KERNEL_BACKUP_INCLUDE_DIRS := $(foreach dir,$(INCLUDE_DIRS),$(abspath $(dir)))
KERNEL_INCLUDE_FLAGS := $(addprefix -I,$(KERNEL_INCLUDE_DIRS))
KERNEL_STYLE_DIRS := $(sort $(KERNEL_SRC_DIR) $(KERNEL_BACKUP_INCLUDE_DIRS))
KERNEL_FIND_PRUNE := \( -type d \( -name .git -o -name bkp -o -name build -o -name dist -o -name 'cmake-build-*' \) -prune \) -o
KERNEL_SOURCE_FILES := -type f \( -name '*.c' -o -name '*.h' -o -name '*.S' -o -name '*.s' \)
KERNEL_STAGE_CLEAN = find "$(KERNEL_BUILD_DIR)" \
	\( -path "$(KERNEL_BUILD_DIR)/bkp" -o -path "$(KERNEL_BUILD_DIR)/dist" \) -prune -o \
	-type f ! -name '*.mod.c' \( -name '*.c' -o -name '*.h' -o -name '*.S' -o -name '*.s' \) \
	-exec rm -f {} +

ifeq ($(KERNELRELEASE),)
ifneq (,$(filter $(KERNEL_BUILD_DIR) $(KERNEL_BUILD_DIR)/%,$(KERNEL_SRC_DIR)))
$(error KERNEL_BUILD_DIR ($(KERNEL_BUILD_DIR)) must not contain KERNEL_SRC_DIR ($(KERNEL_SRC_DIR)))
endif
endif

ifneq ($(strip $(SHELL_DIRS)),)
SHELL_FILES := $(shell find $(abspath $(SHELL_DIRS)) -name "*.sh" 2>/dev/null)
else
SHELL_FILES :=
endif

ifeq ($(V),1)
    LKM_FLAGS += -v
endif
ifeq ($(C),1)
    LKM_FLAGS += -c
endif
ifeq ($(B),1)
    LKM_FLAGS += -b
endif

KERNEL_PKG_NAME  ?= $(KERNEL_DIST_DIR)/$(MOD_NAME_C)

# --- Architecture & Kernel Directory
# Usage:
#   make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu-
KDIR ?= /lib/modules/$(shell uname -r)/build
KMODDIR ?= /lib/modules/$(shell uname -r)

# --- Toolchain
CC    := $(CROSS_COMPILE)gcc
STRIP := $(CROSS_COMPILE)strip

# Keep the existing variable name; builds run from the staged directory.
PWD := $(KERNEL_BUILD_DIR)
obj-m += $(MOD_NAME_C).o

# --- Debug & Compiler Flags
ONDEBUG ?= $(DEBUG_MODE)
DEBUG_MODE_VALUE := $(if $(filter y 1,$(DEBUG_MODE)),1,0)
DBG_STRIP := y

ccflags-y += -DDEBUG_MODE=$(DEBUG_MODE_VALUE)

ifneq ($(filter y 1,$(ONDEBUG)),)
    ccflags-y += -DDEBUG -g -Wall -fno-omit-frame-pointer -ggdb -gdwarf-4 -fvar-tracking-assignments
    DBG_STRIP := n
else
    ccflags-y += -UDEBUG -Wall
endif

ccflags-y += -DDYNAMIC_DEBUG_MODULE
ccflags-y += $(KERNEL_INCLUDE_FLAGS)

# Include kernel configuration settings
-include $(KDIR)/.config

# --- Module Stripping Policy
# --------------------------------------------------------------------------------------
# Quoted from "Linux Kernel Programming - Part 2" by Kaiwan N. Billimoria (Packt):
# "Strip the module? Note:
#  a) Only strip debug symbols else it won't load correctly
#  b) WARNING! Don't strip modules when using CONFIG_MODULE_SIG* cryptographic security"
# --------------------------------------------------------------------------------------
ifdef CONFIG_MODULE_SIG
    DBG_STRIP := n
endif
ifdef CONFIG_MODULE_SIG_ALL
    DBG_STRIP := n
endif
ifdef CONFIG_MODULE_SIG_FORCE
    DBG_STRIP := n
endif

# --- Tools Settings
INDENT       := indent
INDENT_FLAGS := -npro -kr -i8 -ts8 -sob -l80 -ss -ncs -cp1 -il0
CLANG_FORMAT := clang-format
CHECKPATCH   ?= $(if $(wildcard $(KDIR)/scripts/checkpatch.pl),$(KDIR)/scripts/checkpatch.pl,checkpatch.pl)
BEAR         := bear
DTC          := dtc
SPARSE       := sparse
FLAWFINDER   := flawfinder
CPPCHECK     := cppcheck
TAR          := tar
SHFMT        ?= shfmt
SHELLCHECK   ?= shellcheck
LKM          := $(abspath $(SCRIPTS_DIR)/lkm.sh)

# Drop the leading '@' at the beginning of a macro.
noat = $(subst @,,$(1))

# Older kbuild versions require external-module sources beside the output
# directory. Stage them only while kbuild runs, then remove them on success or
# failure so build/kernel retains generated files only.
define run_kbuild
@set -e; \
	clean_stage() { $(KERNEL_STAGE_CLEAN); }; \
	trap clean_stage EXIT; \
	(cd "$(KERNEL_SRC_DIR)" && find . $(KERNEL_FIND_PRUNE) \
		-type f ! -name '*.mod.c' $(KERNEL_SOURCE_FILES) \
		-exec cp --parents -t "$(KERNEL_BUILD_DIR)" {} +); \
	$(1)
endef

# ======
# BUILD
# ======
.PHONY: all build run install dt nsdeps clean compile-bear lkcode-style \
	format clang-format indent backup-sources checkpatch format-shell lint-shell sa sa_sparse sa_gcc sa_flawfinder \
	sa_cppcheck dist tarxz-pkg clean-dist mrproper help env

ifeq ($(KERNELRELEASE),)
.DEFAULT_GOAL := help
ifeq ($(MOD_NAME_C),)
    $(error ERROR: You must pass the C file like this: make MOD_NAME_C=filename target)
else ifneq ("$(origin MOD_NAME_C)", "command line")
    $(info Name of module: $(MOD_NAME_C))
endif
endif

all: clean lkcode-style sa compile-bear
.NOTPARALLEL: all

build: env
	$(call print_step,Building '$(MOD_NAME_C).ko' [ARCH=$(ARCH) KDIR=$(KDIR)])
	$(call find_package,$(MAKE))
	$(call run_kbuild,$(MAKE) -C "$(KDIR)" M="$(KERNEL_BUILD_DIR)" ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) INCLUDE_DIRS="$(KERNEL_INCLUDE_DIRS)" modules)
	@if [ -f "$(KERNEL_BUILD_DIR)/$(MOD_NAME_C).ko" ]; then \
		$(call noat,$(call print_info,Module $(MOD_NAME_C).ko built successfully.)); \
	else \
		$(call noat,$(call print_err,Failed to build $(MOD_NAME_C).ko)); \
		exit 1; \
	fi

run: env
	$(call print_step, run the $(LKM) script)
	$(Q)cd "$(KERNEL_BUILD_DIR)" && sudo "$(LKM)" $(LKM_FLAGS) -r "$(MOD_NAME_C)"

install: env
	$(call print_step,Installing module to $(KMODDIR)...)
	@echo
	$(call print_g,     Invoking the kernel build)
	$(call find_package,$(MAKE))
	$(call run_kbuild,$(MAKE) -C "$(KDIR)" M="$(KERNEL_BUILD_DIR)" ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) INCLUDE_DIRS="$(KERNEL_INCLUDE_DIRS)" modules)
	@echo
	$(call print_g,     Invoking sudo make modules_install)
	$(Q)sudo $(MAKE) -C "$(KDIR)" M="$(KERNEL_BUILD_DIR)" INCLUDE_DIRS="$(KERNEL_INCLUDE_DIRS)" modules_install
	$(Q)sudo depmod
	$(call find_package,$(STRIP))
	@if [ "$(DBG_STRIP)" = "y" ]; then \
		$(call noat,$(call print_g,     Stripping debug info from $(KMODDIR)/extra/$(MOD_NAME_C).ko)); \
		sudo $(STRIP) --strip-debug $(KMODDIR)/extra/$(MOD_NAME_C).ko; \
	fi

dt: env
	$(call find_package,$(DTC))
	$(call print_step,Compiling Device Tree Blob from $(MOD_NAME_C).dts...)
	$(Q)$(DTC) -@ -I dts -O dtb \
		$(foreach dir,$(KERNEL_INCLUDE_DIRS),-i $(dir)) \
		-o $(KERNEL_BUILD_DIR)/$(MOD_NAME_C).dtb \
		$(KERNEL_SRC_DIR)/$(MOD_NAME_C).dts

nsdeps: env
	$(call print_step,Resolving namespace dependencies...)
	$(call find_package,$(MAKE))
	$(call run_kbuild,$(MAKE) -C "$(KDIR)" M="$(KERNEL_BUILD_DIR)" INCLUDE_DIRS="$(KERNEL_INCLUDE_DIRS)" nsdeps)

env: $(KERNEL_ENV_STAMP)

$(KERNEL_ENV_STAMP): $(KERNEL_ENV_FILES)
	$(call print_step,Creating kernel module build environment)
	$(Q)$(BUILDER) $(BUILDER_FLAGS) --create --directory "$(KERNEL_BUILD_DIR)"
	$(Q)$(BUILDER) $(BUILDER_FLAGS) --create --directory "$(KERNEL_DIST_DIR)"
	$(Q)$(BUILDER) $(BUILDER_FLAGS) --copy --files "$(ROOT_DIR)/Makefile" --directory "$(BUILD_DIR)"
	$(Q)$(BUILDER) $(BUILDER_FLAGS) --copy --files "$(KERNEL_SRC_DIR)/Makefile" --directory "$(KERNEL_BUILD_DIR)"
	$(Q)$(KERNEL_STAGE_CLEAN)
	@if [ -f "$(KERNEL_SRC_DIR)/Kbuild" ]; then \
		cp -f "$(KERNEL_SRC_DIR)/Kbuild" "$(KERNEL_BUILD_DIR)/Kbuild"; \
	fi
	@if [ -f "$(KERNEL_SRC_DIR)/Kconfig" ]; then \
		cp -f "$(KERNEL_SRC_DIR)/Kconfig" "$(KERNEL_BUILD_DIR)/Kconfig"; \
	fi
	$(Q)touch "$@"

# =======================
# LINUX KERNEL CODE STYLE
# =======================
format: lkcode-style
lkcode-style: clang-format indent checkpatch format-shell

backup-sources:
	$(Q)mkdir -p "$(KERNEL_BUILD_DIR)/bkp"
	$(Q)for dir in $(KERNEL_STYLE_DIRS); do \
		rel_dir=$$(realpath --relative-to="$(ROOT_DIR)" "$$dir"); \
		case "$$rel_dir" in ../*) rel_dir="external/$$(basename "$$dir")" ;; esac; \
		mkdir -p "$(KERNEL_BUILD_DIR)/bkp/$$rel_dir"; \
		(cd "$$dir" && find . $(KERNEL_FIND_PRUNE) $(KERNEL_SOURCE_FILES) \
			-exec cp --parents -t "$(KERNEL_BUILD_DIR)/bkp/$$rel_dir" {} +); \
	done

clang-format: backup-sources
	$(call find_package,$(CLANG_FORMAT))
	$(call print_step,Formatting kernel source files with clang-format)
	$(Q)find $(KERNEL_STYLE_DIRS) $(KERNEL_FIND_PRUNE) \
		-type f \( -name '*.c' -o -name '*.h' \) \
		-exec $(CLANG_FORMAT) --style=file --fallback-style=none -i {} +

indent: backup-sources
	$(call find_package,$(INDENT))
	$(call print_step,Applying kernel code style indentation)
	$(Q)find $(KERNEL_STYLE_DIRS) \
		$(KERNEL_FIND_PRUNE) $(KERNEL_SOURCE_FILES) \
		-exec sh -c '\
			indent_bin=$$1; shift; \
			for source do \
				temp="$${source}.indent.tmp"; \
				cp -p "$$source" "$$temp" || exit 1; \
				if "$$indent_bin" $(INDENT_FLAGS) -o "$$temp" "$$source"; then \
					mv -f "$$temp" "$$source" || exit 1; \
				else \
					rm -f "$$temp"; exit 1; \
				fi; \
			done' sh "$(INDENT)" {} +

checkpatch:
	$(call find_package,$(CHECKPATCH))
	$(call print_step,Running kernel code style check with checkpatch.pl)
	$(Q)find $(KERNEL_STYLE_DIRS) $(KERNEL_FIND_PRUNE) \
		-type f \( -name '*.c' -o -name '*.h' \) -print0 | \
		xargs -0 -r $(CHECKPATCH) --no-tree -f --max-line-length=95

ifneq ($(strip $(SHELL_FILES)),)
format-shell: lint-shell
	$(call find_package,$(SHFMT))
	$(call print_step,Formatting shell scripts with $(SHFMT))
	$(Q)$(SHFMT) -w -i 4 -fn $(SHELL_FILES)

lint-shell:
	$(call find_package,$(SHELLCHECK))
	$(call print_step,Linting shell scripts (POSIX sh) with $(SHELLCHECK))
	$(Q)$(SHELLCHECK) -s sh -x $(SHELL_FILES)

else
format-shell lint-shell:
	$(call print_y,No shell scripts found; skipping shell lint and format.)
endif

# =====================
# SA - STATIC ANALYSIS
# =====================
sa: sa_sparse sa_gcc sa_flawfinder sa_cppcheck

sa_sparse: env
	$(call find_package,$(SPARSE))
	$(call find_package,$(MAKE))
	$(call print_step,Running static analysis with sparse)
	$(call run_kbuild,set +e; log="$(KERNEL_BUILD_DIR)/.sparse.log"; $(MAKE) -C "$(KDIR)" M="$(KERNEL_BUILD_DIR)" C=2 CHECK="$(SPARSE) --os=linux --arch=$(ARCH)" INCLUDE_DIRS="$(KERNEL_INCLUDE_DIRS)" modules > "$$log" 2>&1; status=$$?; grep -Ev "^\./include/.*\.h|^\./arch/.*\.h" "$$log" || true; rm -f "$$log"; exit "$$status")

sa_gcc: env
	$(call find_package,$(MAKE))
	$(call print_step,Running static analysis with GCC W=1)
	$(call run_kbuild,$(MAKE) W=1 -C "$(KDIR)" M="$(KERNEL_BUILD_DIR)" INCLUDE_DIRS="$(KERNEL_INCLUDE_DIRS)" modules)

sa_flawfinder:
	$(call find_package,$(FLAWFINDER))
	$(call print_step,Running static analysis with flawfinder)
	$(Q)$(FLAWFINDER) $(KERNEL_STYLE_DIRS)

sa_cppcheck:
	$(call find_package,$(CPPCHECK))
	$(call print_step,Running static analysis with cppcheck)
	$(Q)$(CPPCHECK) -v --force --enable=all \
		$(KERNEL_INCLUDE_FLAGS) \
		-i "$(KERNEL_BUILD_DIR)" -i "$(KERNEL_DIST_DIR)" \
		--suppress=missingIncludeSystem $(KERNEL_STYLE_DIRS)

# =================
# BUILD ENVIRONMENT
# =================
compile-bear: env
	$(call find_package,$(BEAR))
	$(call find_package,$(MAKE))
	$(call print_step,Generating compile_commands.json with bear...)
	$(call run_kbuild,cd "$(KERNEL_BUILD_DIR)" && $(BEAR) -- $(MAKE) -C "$(KDIR)" M="$(KERNEL_BUILD_DIR)" ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) INCLUDE_DIRS="$(KERNEL_INCLUDE_DIRS)" modules)
	@if [ -f "$(KERNEL_BUILD_DIR)/$(MOD_NAME_C).ko" ]; then \
		$(call noat,$(call print_info,Module '$(MOD_NAME_C).ko' built and compile_commands.json generated.)); \
	fi

# ================
# ARCHIVE WITH TAR
# ================
dist: tarxz-pkg

tarxz-pkg: clean env
	$(call find_package,$(TAR))
	$(Q)rm -f "$(KERNEL_PKG_NAME).tar.xz" 2>/dev/null || true
	$(call print_step,Packaging source archive)
	$(Q)$(TAR) --exclude='./.git' --exclude='./build' --exclude='./dist' \
		-caf "$(KERNEL_PKG_NAME).tar.xz" -C "$(KERNEL_SRC_DIR)" .
	$(Q)ls -lh "$(KERNEL_PKG_NAME).tar.xz"
	$(call print_g,Package created: $(KERNEL_PKG_NAME).tar.xz)

clean-dist:
	$(call print_step,Removing generated kernel archives)
	$(Q)rm -f "$(KERNEL_DIST_DIR)"/*.tar.gz "$(KERNEL_DIST_DIR)"/*.tar.xz \
		"$(KERNEL_DIST_DIR)"/*.tar.bz2 2>/dev/null || true

clean:
	$(call print_step,Cleaning kernel build artifacts)
	@if [ -d "$(KERNEL_BUILD_DIR)" ] && [ -d "$(KDIR)" ]; then \
		$(MAKE) -C "$(KDIR)" M="$(KERNEL_BUILD_DIR)" clean; \
	fi
	$(Q)rm -f $(patsubst %,$(KERNEL_BUILD_DIR)/%,*~ compile_commands.json *.o *.ko *.mod *.mod.c *.symvers *.order *.dtb)
	$(call print_g,Cleaning finished.)

mrproper: clean clean-dist
	$(call print_step,Removing kernel build directory)
	$(Q)rm -rf "$(KERNEL_BUILD_DIR)"

# ==============
# MAKE HELP MENU
# ==============
help:
	@echo
	$(call print_g,=== Kernel Makefile Help ===)
	$(call print_y,Usage: make <target> [VARIABLE=value])
	@echo
	$(call print_g,Build & Execution Targets ::)
	@echo "  build          Build the module and leave generated artifacts under $(KERNEL_BUILD_DIR)"
	@echo "  run            Build if needed, then load the module (requires root)"
	@echo "  install        Build, install the module, and run depmod (requires root)"
	@echo "  dt             Compile a Device Tree Blob from the matching .dts file"
	@echo "  nsdeps         Resolve module symbol namespace dependencies"
	@echo "  env            Prepare build directories/Makefiles once, and refresh when their sources change"
	@echo "  compile-bear   Build the module and generate compile_commands.json with Bear"
	@echo "  clean          Remove generated module/build artifacts (keeps bkp/ and dist/)"
	@echo "  all            Run clean, formatting, static analysis, and Bear build"
	@echo "  help           Display this help message"
	@echo
	$(call print_g,Code Style & Quality Targets ::)
	@echo "  format         Run kernel and shell formatting targets"
	@echo "  backup-sources Back up kernel sources and configured headers under $(KERNEL_BUILD_DIR)/bkp"
	@echo "  clang-format   Format C/header files with clang-format"
	@echo "  indent         Format C/header/assembly files with Linux Lindent options"
	@echo "  checkpatch     Check kernel and configured header sources with checkpatch.pl"
	@echo "  lkcode-style   Run clang-format, indent, checkpatch, and shell formatting"
	@echo "  format-shell   Format shell scripts after linting them"
	@echo "  lint-shell     Lint shell scripts with shellcheck"
	@echo
	$(call print_g,Static Analysis Targets ::)
	@echo "  sa             Run all static analyzers"
	@echo "  sa_sparse      Run sparse analysis"
	@echo "  sa_gcc         Run GCC extra warnings (W=1)"
	@echo "  sa_flawfinder  Run flawfinder"
	@echo "  sa_cppcheck    Run cppcheck"
	@echo
	$(call print_g,Packaging & Distribution Targets ::)
	@echo "  dist           Create a source archive in $(KERNEL_DIST_DIR)"
	@echo "  tarxz-pkg      Create a .tar.xz source archive"
	@echo "  clean-dist     Remove generated archives"
	@echo "  mrproper       Remove kernel build files and archives"
	@echo
	$(call print_g,Variables ::)
	@echo "  V               Verbose command output (1 = on, default: 0)"
	@echo "  C               Colored output (1 = on, default: 0)"
	@echo "  B               Bold output (1 = on, default: 0)"
	@echo "  MOD_NAME_C      Module source name without .c extension (default: fanx)"
	@echo "  ARCH            Target architecture (e.g., arm64, powerpc)"
	@echo "  CROSS_COMPILE   Cross-compiler toolchain prefix"
	@echo "  KDIR            Kernel build tree (default: /lib/modules/\$$(uname -r)/build)"
	@echo "  KMODDIR         Module installation root (default: /lib/modules/\$$(uname -r))"
	@echo "  INCLUDE_DIRS    Extra API/header directories (default: ROOT_DIR/include and include/uapi)"
	@echo "  KERNEL_BUILD_DIR Module output directory (default: BUILD_DIR/kernel)"
	@echo "  KERNEL_DIST_DIR Source archive directory (default: KERNEL_BUILD_DIR/dist)"
	@echo "  DEBUG_MODE      C macro value: 1 for y/1, otherwise 0 (default: $(DEBUG_MODE))"
	@echo "  ONDEBUG         Enable debug compiler flags and disable stripping (y/1; default: DEBUG_MODE)"
	@echo "  CHECKPATCH      Kernel checkpatch.pl path or command"
	@echo "  INDENT          GNU indent executable (default: indent)"
	@echo "  INDENT_FLAGS    GNU indent options (Linux Lindent defaults)"
	@echo "  CLANG_FORMAT    Formatter command (default: clang-format)"
	@echo "  BEAR            Compilation database tool (default: bear)"
	@echo "  DTC             Device Tree compiler (default: dtc)"
	@echo "  SPARSE          Sparse analyzer (default: sparse)"
	@echo "  FLAWFINDER      Flawfinder analyzer (default: flawfinder)"
	@echo "  CPPCHECK        Cppcheck analyzer (default: cppcheck)"
	@echo "  SHFMT           Shell formatter (default: shfmt)"
	@echo "  SHELLCHECK      Shell linter (default: shellcheck)"
	@echo