# SPDX-License-Identifier: GPL-2.0

# =============
# CONFIGURATION
# =============
ROOT_DIR   := $(CURDIR)
BUILD_DIR  := $(ROOT_DIR)/build
DRIVER_DIR := $(ROOT_DIR)/kernel
APP_DIR    := $(ROOT_DIR)/app

# Outputs
COLOR   ?= 1
IN_BOLD ?= 0

# Build Parameters
DEBUG_MODE ?= 1
#TODO: implement the kernel macro $(Q)

VERSION ?= $(shell cat $(ROOT_DIR)/VERSION 2>/dev/null || echo "0.0.0-unknown")

export ROOT_DIR VERSION BUILD_DIR COLOR IN_BLOD DEBUG_MODE

include $(ROOT_DIR)/build-aux/utils.mk

# =====
# BUILD
# =====
.PHONY: all help \
        driver app \
        build build-driver build-app \
        format format-driver format-app \
        sa sa-driver sa-app \
        clean clean-driver clean-app

.DEFAULT_GOAL := help

print-version:
	@echo -n "Project Version: "
	$(call print_b,$(VERSION))

# Sequential on purpose, even with make -j
all:
	$(MAKE) clean
	$(MAKE) format
	$(MAKE) build

# Print kernelspace module (driver) help
driver:
	@$(MAKE) -C $(DRIVER_DIR) help
	@$(call print_y,To run a driver target from the root directory ($(ROOT_DIR)): make driver-<target> or make -C kernel <targets>)

# Print userspace application help
app:
	@$(MAKE) -C $(APP_DIR) help
	@$(call print_y,To run an app target from the root directory ($(ROOT_DIR)): make app-<target> or make -C app <targets>)

app-%:
	@$(MAKE) -C $(APP_DIR) $*

driver-%:
	@$(MAKE) -C $(DRIVER_DIR) $*

build: build-driver build-app

build-driver:
	$(call print_step,Building kernel driver.)
	$(MAKE) -C $(DRIVER_DIR) build

build-app:
	$(call print_step,Building userspace application.)
	$(MAKE) -C $(APP_DIR) build

format: format-driver format-app

format-driver:
	$(call print_step,Formatting kernel driver source code (.c, .h) files.)
	$(MAKE) -C $(DRIVER_DIR) format

format-app:
	$(call print_step,Formatting userspace source code (.c, .cpp, .h) files.)
	$(MAKE) -C $(APP_DIR) format

sa: sa-driver sa-app

sa-driver:
	$(call print_step,Running static analysis on the kernel driver.)
	$(MAKE) -C $(DRIVER_DIR) sa

sa-app:
	$(call print_step,Running static analysis on the userspace application.)
	$(MAKE) -C $(APP_DIR) sa

clean: clean-driver clean-app

clean-driver:
	$(call print_step,Cleaning kernel driver build artifacts.)
	$(MAKE) -C $(DRIVER_DIR) clean

clean-app:
	$(call print_step,Cleaning userspace build artifacts.)
	$(MAKE) -C $(APP_DIR) clean

# =========
# HELP MENU
# =========
help:
	@echo
	$(call print_g,=== Top-Level Makefile Help ===)
	$(call print_y,Usage: make <target> [VARIABLE=value])
	@echo
	$(call print_g,Build and Orchestration Targets ::)
	@echo "  all             Run clean, format, and build all targets (in this order)"
	@echo "  build           Build both kernel driver and userspace application"
	@echo "  build-driver    Build the kernel driver only"
	@echo "  build-app       Build the userspace application only"
	@echo "  driver          Print kernelspace module (driver) Makefile 'help' target ($(DRIVER_DIR))"
	@echo "  app             Print userspace application Makefile 'help' target ($(APP_DIR))"
	@echo "  app-<target>    Run app targets, see 'make app'"
	@echo "  driver-<target> Run driver targets, see 'make driver'"
	@echo "  clean           Remove build artifacts across the project"
	@echo "  print-version   Print project version"
	@echo "  clean-driver    Remove kernel driver build artifacts"
	@echo "  clean-app       Remove userspace build artifacts"
	@echo "  help            Display this help message"
	@echo ""
	$(call print_g,Code Style and Quality Targets ::)
	@echo "  format          Format all source code (.c, .cpp, .h) files"
	@echo "  format-driver   Format the kernel driver source code"
	@echo "  format-app      Format the userspace source code"
	@echo "  sa              Run static analyzers for app and driver"
	@echo "  sa-driver       Run static analyzers for the driver only"
	@echo "  sa-app          Run static analyzers for the app only"
	@echo ""
	$(call print_g,Variables ::)
	@echo "  COLOR           Enable/disable colors (default: 1)"
	@echo "  IN_BOLD         Enable/disable bold text (default: 0)"
	@echo "  DEBUG_MODE      Enable debug mode (default: 1)"
	@echo "  ROOT_DIR        Current root directory of the project"
	@echo "  ARCH            Target architecture, forwarded to the kernel build"
	@echo "  CROSS_COMPILE   Cross-compiler prefix, forwarded to both builds"
	@echo "  KDIR            Kernel build tree used by the driver"