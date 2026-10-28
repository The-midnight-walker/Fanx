# SPDX-License-Identifier: GPL-2.0
# vim: set ts=8 sw=8 noet tw=80 cc=80 fo+=t :

# Shared Makefiles Configuration
# ==============================

# Define the debug mode
DEBUG_MODE := 0

# Determine the project root directory from CONFIG_DIR
ROOT_DIR := $(shell realpath --relative-to="$(CURDIR)" "$(CONFIG_DIR)/..")
CONFIG_DIR := $(ROOT_DIR)/build-aux

VERSION     := $(shell cat $(ROOT_DIR)/VERSION 2>/dev/null || echo "0.0.0-unknown")
DRIVER_DIR  := $(ROOT_DIR)/kernel
APP_DIR     := $(ROOT_DIR)/app
SCRIPTS_DIR := $(ROOT_DIR)/scripts

# ========
# BUILDING
# ========
BUILDER       := /bin/sh $(CONFIG_DIR)/builder.sh
BUILDER_FLAGS :=

# Build directories
BUILD_DIR     := $(ROOT_DIR)/build
APP_BUILD_DIR := $(BUILD_DIR)/app
DRV_BUILD_DIR := $(BUILD_DIR)/kernel

# =============
# OUTPUT STYLES
# =============
COLOR   := 1
VERBOSE := 1
IN_BOLD := 0

ifeq ($(COLOR),1)
    BUILDER_FLAGS += --color
endif
ifeq ($(VERBOSE),1)
    BUILDER_FLAGS += --verbose
endif
ifeq ($(IN_BOLD),1)
    BUILDER_FLAGS += --bold
endif

# Package and tool verification macro
define find_package
	@command -v $(1) >/dev/null 2>&1 || { $(BUILDER) $(BUILDER_FLAGS) --print red "$(1) is not installed on your host" >&2; exit 1; }
endef

define print_err
	@$(BUILDER) $(BUILDER_FLAGS) --print red "$(1)" >&2
endef

define print_info
	@$(BUILDER) $(BUILDER_FLAGS) --print yellow "$(1)"
endef

define print_step
	@echo -n "===> "
	@$(BUILDER) $(BUILDER_FLAGS) --print blue "$(1)"
endef

define print_y
	@$(BUILDER) $(BUILDER_FLAGS) --print yellow "$(1)"
endef

define print_g
	@$(BUILDER) $(BUILDER_FLAGS) --print green "$(1)"
endef

define print_b
	@$(BUILDER) $(BUILDER_FLAGS) --print blue "$(1)"
endef
