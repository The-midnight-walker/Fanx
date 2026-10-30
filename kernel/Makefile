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
SHELL_DIRS   ?= $(SCRIPTS_DIR) $(CONFIG_DIR)
INCLUDE_DIRS ?= $(ROOT_DIR)/include
KERNEL_INCLUDE_DIRS := $(foreach dir,$(INCLUDE_DIRS),$(abspath $(dir)))
KERNEL_INCLUDE_FLAGS := $(addprefix -I,$(KERNEL_INCLUDE_DIRS))

SHELL_FILES  := $(shell find $(SHELL_DIRS) -name "*.sh" 2>/dev/null)
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
ifeq ($(DEBUG_MODE),)
    ONDEBUG := y
endif
DBG_STRIP := y

ifeq ($(ONDEBUG),y)
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

# Drop the leading '@' at the beginning of a macro.
noat = $(subst @,,$(1))

# Older kbuild versions require external-module sources beside the output
# directory. Stage them only while kbuild runs, then remove them on success or
# failure so build/kernel retains generated files only.
define run_kbuild
@set -e; \
	trap 'find "$(KERNEL_BUILD_DIR)" -type f ! -name "*.mod.c" \( -name "*.c" -o -name "*.h" -o -name "*.S" -o -name "*.s" \) -delete' EXIT; \
	(cd "$(KERNEL_SRC_DIR)" && find . -type d \( -name .git -o -name bkp -o -name dist -o -name build -o -name 'cmake-build-*' \) -prune -o \
		-type f ! -name '*.mod.c' \( -name '*.c' -o -name '*.h' -o -name '*.S' -o -name '*.s' \) \
		-exec cp --parents -t "$(KERNEL_BUILD_DIR)" {} +); \
	$(1)
endef

# ======
# BUILD
# ======
.PHONY: all build install dt nsdeps clean compile-bear lkcode-style \
	clang-format indent checkpatch format-shell lint-shell sa sa_sparse sa_gcc sa_flawfinder \
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

env:
	$(call print_step,Creating kernel module build environment)
	$(Q)$(BUILDER) $(BUILDER_FLAGS) --create --directory "$(KERNEL_BUILD_DIR)"
	$(Q)$(BUILDER) $(BUILDER_FLAGS) --create --directory "$(KERNEL_DIST_DIR)"
	$(Q)$(BUILDER) $(BUILDER_FLAGS) --copy --files "$(KERNEL_SRC_DIR)/Makefile" --directory "$(KERNEL_BUILD_DIR)"
	$(Q)find "$(KERNEL_BUILD_DIR)" -type f ! -name '*.mod.c' \( -name '*.c' -o -name '*.h' -o -name '*.S' -o -name '*.s' \) -delete
	@if [ -f "$(KERNEL_SRC_DIR)/Kbuild" ]; then \
		cp -f "$(KERNEL_SRC_DIR)/Kbuild" "$(KERNEL_BUILD_DIR)/Kbuild"; \
	fi
	@if [ -f "$(KERNEL_SRC_DIR)/Kconfig" ]; then \
		cp -f "$(KERNEL_SRC_DIR)/Kconfig" "$(KERNEL_BUILD_DIR)/Kconfig"; \
	fi

# =======================
# LINUX KERNEL CODE STYLE
# =======================
format: lkcode-style
lkcode-style: clang-format indent checkpatch format-shell

clang-format:
	$(call find_package,$(CLANG_FORMAT))
	$(call print_step,Formatting kernel source files with clang-format)
	$(Q)find "$(KERNEL_SRC_DIR)" $(KERNEL_INCLUDE_DIRS) \
		\( -type d \( -name .git -o -name bkp -o -name build -o -name dist \
			-o -name 'cmake-build-*' \) -prune \) -o \
		-type f \( -name '*.c' -o -name '*.h' \) \
		-exec $(CLANG_FORMAT) --fallback-style=none -i {} +

indent:
	$(call find_package,$(INDENT))
	$(call print_step,Applying kernel code style indentation)
	$(Q)mkdir -p "$(KERNEL_BUILD_DIR)/bkp"
	$(Q)cp -f "$(KERNEL_SRC_DIR)"/*.[chsS] "$(KERNEL_BUILD_DIR)/bkp/" 2>/dev/null || true
	$(Q)find "$(KERNEL_SRC_DIR)" $(KERNEL_INCLUDE_DIRS) \
		\( -type d \( -name .git -o -name bkp -o -name build -o -name dist \
			-o -name 'cmake-build-*' \) -prune \) -o \
		-type f \( -name '*.c' -o -name '*.h' -o -name '*.S' -o -name '*.s' \) \
		-exec $(INDENT) -linux --line-length95 {} + 2>/dev/null || true

checkpatch:
	$(call find_package,$(CHECKPATCH))
	$(call print_step,Running kernel code style check with checkpatch.pl)
	$(Q)find "$(KERNEL_SRC_DIR)" $(KERNEL_INCLUDE_DIRS) \
		-type f \( -name '*.c' -o -name '*.h' \) -print0 | \
		xargs -0 -r $(CHECKPATCH) --no-tree -f --max-line-length=95

format-shell: lint-shell
	$(call find_package,$(SHFMT))
	$(call print_step,Formatting shell scripts with $(SHFMT))
	$(Q)$(SHFMT) -w -i 4 -fn $(SHELL_FILES)

lint-shell:
	$(call find_package,$(SHELLCHECK))
	$(call print_step,Linting shell scripts (POSIX sh) with $(SHELLCHECK))
	$(Q)$(SHELLCHECK) -s sh -x $(SHELL_FILES)

# =====================
# SA - STATIC ANALYSIS
# =====================
sa: sa_sparse sa_gcc sa_flawfinder sa_cppcheck

sa_sparse: env
	$(call find_package,$(SPARSE))
	$(call find_package,$(MAKE))
	$(call print_step,Running static analysis with sparse)
	$(call run_kbuild,$(MAKE) -C "$(KDIR)" M="$(KERNEL_BUILD_DIR)" C=2 CHECK="$(SPARSE) --os=linux --arch=$(ARCH)" INCLUDE_DIRS="$(KERNEL_INCLUDE_DIRS)" modules 2>&1 | egrep -v "^\./include/.*\.h|^\./arch/.*\.h")

sa_gcc: env
	$(call find_package,$(MAKE))
	$(call print_step,Running static analysis with GCC W=1)
	$(call run_kbuild,$(MAKE) W=1 -C "$(KDIR)" M="$(KERNEL_BUILD_DIR)" INCLUDE_DIRS="$(KERNEL_INCLUDE_DIRS)" modules)

sa_flawfinder:
	$(call find_package,$(FLAWFINDER))
	$(call print_step,Running static analysis with flawfinder)
	$(Q)$(FLAWFINDER) "$(KERNEL_SRC_DIR)" $(KERNEL_INCLUDE_DIRS)

sa_cppcheck:
	$(call find_package,$(CPPCHECK))
	$(call print_step,Running static analysis with cppcheck)
	$(Q)$(CPPCHECK) -v --force --enable=all \
		$(KERNEL_INCLUDE_FLAGS) \
		-i "$(KERNEL_BUILD_DIR)" -i "$(KERNEL_DIST_DIR)" \
		--suppress=missingIncludeSystem "$(KERNEL_SRC_DIR)" $(KERNEL_INCLUDE_DIRS)

# =================
# BUILD ENVIRONMENT
# =================
compile-bear: env
	$(call find_package,$(BEAR))
	$(call find_package,$(MAKE))
	$(call print_step,Generating compile_commands.json with bear...)
	$(call run_kbuild,cd "$(KERNEL_BUILD_DIR)" && $(BEAR) -- $(MAKE) -C "$(KDIR)" M="$(KERNEL_BUILD_DIR)" ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) INCLUDE_DIRS="$(KERNEL_INCLUDE_DIRS)" modules)
	@if [ -f "$(KERNEL_BUILD_DIR)/$(MOD_NAME_C).ko" ]; then \
		$(call noat,$(call print_info,Module built and compile_commands.json generated.)); \
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
	$(call print_y,Usage: make <target> [MOD_NAME_C=filename] [ARCH=arch] [CROSS_COMPILE=prefix])
	@echo
	$(call print_g,Build & Execution Targets ::)
	@echo "  build          Build the kernel module (.ko)"
	@echo "  install        Install the module and run depmod"
	@echo "  dt             Compile a Device Tree Blob from the matching .dts file"
	@echo "  nsdeps         Resolve module symbol namespace dependencies"
	@echo "  env            Prepare $(KERNEL_BUILD_DIR) and copy its build Makefiles"
	@echo "  compile-bear   Build the module and generate compile_commands.json"
	@echo "  clean          Remove generated files from $(KERNEL_BUILD_DIR)"
	@echo "  all            Run clean, style checks, static analysis, and bear build"
	@echo "  help           Display this help message"
	@echo
	$(call print_g,Code Style & Quality Targets ::)
	@echo "  clang-format   Format kernel C sources"
	@echo "  indent         Apply Linux kernel indentation"
	@echo "  checkpatch     Check source files with checkpatch.pl"
	@echo "  lkcode-style   Run formatting and checkpatch targets"
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
	@echo "  V               Kernel-style verbose flag (1 = show commands, default: 0)"
	@echo "  C               Enable/disable colors (default: 1)"
	@echo "  B               Enable/disable bold text (default: 0)"
	@echo "  MOD_NAME_C      Module source name without .c extension"
	@echo "  ARCH            Target architecture (e.g., arm64, powerpc)"
	@echo "  CROSS_COMPILE   Cross-compiler toolchain prefix"
	@echo "  KDIR            Kernel build tree directory"
	@echo "  KMODDIR         Module installation directory"
	@echo "  INCLUDE_DIRS    Additional API/header directories"
	@echo "  KERNEL_BUILD_DIR Build output directory (default: BUILD_DIR/kernel)"
	@echo "  KERNEL_DIST_DIR Source archive directory (default: BUILD_DIR/kernel/dist)"
	@echo "  ONDEBUG         Enable debug flags and disable stripping (y/n)"
	@echo "  CHECKPATCH      Path to kernel checkpatch.pl script"
	@echo "  INDENT          Formatter binary (default: indent)"
	@echo "  CPPCHECK        Cppcheck binary (default: cppcheck)"
	@echo