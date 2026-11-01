# SPDX-License-Identifier: GPL-2.0

CONFIG_DIR ?= $(shell \
	dir="$(CURDIR)"; \
	while [ "$$dir" != "/" ]; do \
		if [ -d "$$dir/build-aux" ]; then \
			realpath --relative-to="$(CURDIR)" "$$dir/build-aux"; \
			break; \
		fi; \
		dir=$$(dirname "$$dir"); \
	done \
)

include $(CONFIG_DIR)/config.mk

# =====
# BUILD
# =====
.PHONY: all help print-version \
        driver app \
        build build-driver build-app \
        format format-driver format-app \
        sa sa-driver sa-app \
        clean clean-driver clean-app \
        run run-driver run-app \
        mrproper env app-env driver-env

.DEFAULT_GOAL := help
.NOTPARALLEL: all

print-version:
	@echo -n "Project Version: "
	$(call print_b,$(VERSION))

# Sequential on purpose, even with make -j
all: clean format build

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
	$(call print_step,Building kernel driver)
	$(MAKE) -C $(DRIVER_DIR) build

build-app:
	$(call print_step,Building userspace application)
	$(MAKE) -C $(APP_DIR) build

format: format-driver format-app

format-driver:
	$(call print_step,Formatting kernel driver source code)
	$(MAKE) -C $(DRIVER_DIR) format

format-app:
	$(call print_step,Formatting userspace source code)
	$(MAKE) -C $(APP_DIR) format

sa: sa-driver sa-app

sa-driver:
	$(call print_step,Running static analysis on kernel driver)
	$(MAKE) -C $(DRIVER_DIR) sa

sa-app:
	$(call print_step,Running static analysis on userspace application)
	$(MAKE) -C $(APP_DIR) sa

clean: clean-driver clean-app

clean-driver:
	$(call print_step,Cleaning kernel driver build artifacts)
	$(MAKE) -C $(DRIVER_DIR) clean

clean-app:
	$(call print_step,Cleaning userspace build artifacts)
	$(MAKE) -C $(APP_DIR) clean

run: run-app run-driver

run-driver:
	$(MAKE) -C $(DRIVER_DIR) run

run-app:
	@$(call print_y,run-app: not implemented yet)

mrproper:
	$(call print_step,Clearing entire build environment)
	rm -rf $(BUILD_DIR)

env: app-env driver-env

app-env:
	$(MAKE) -C $(APP_DIR) env

driver-env:
	$(MAKE) -C $(DRIVER_DIR) env

# =========
# HELP MENU
# =========
help:
	@echo
	$(call print_g,=== Top-Level Makefile Help ===)
	$(call print_y,Usage: make <target> [VARIABLE=value])
	@echo
	$(call print_g,Build and Orchestration Targets ::)
	@echo "  all             Run clean, format, then build app and driver sequentially"
	@echo "  build           Build both kernel driver and userspace application"
	@echo "  build-driver    Build the kernel driver only"
	@echo "  build-app       Build the userspace application only"
	@echo "  env             Prepare build environment for app and driver"
	@echo "  driver          Print kernelspace module (driver) Makefile 'help' target ($(DRIVER_DIR))"
	@echo "  app             Print userspace application Makefile 'help' target ($(APP_DIR))"
	@echo "  app-<target>    Run app targets, see 'make app'"
	@echo "  driver-<target> Run driver targets, see 'make driver'"
	@echo "  clean           Remove build artifacts across the project"
	@echo "  clean-driver    Remove kernel driver build artifacts"
	@echo "  clean-app       Remove userspace build artifacts"
	@echo "  mrproper        Completely remove the build directory ($(BUILD_DIR))"
	@echo "  run             Run the userspace app and driver test targets (currently stubs)"
	@echo "  print-version   Print project version"
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
	@echo "  V               Kernel-style verbose flag (1 = show commands, default: 0)"
	@echo "  C               Enable/disable colors (default: 1)"
	@echo "  B               Enable/disable bold text (default: 0)"
	@echo "  IN_BOLD         Enable/disable bold text (default: 0)"
	@echo "  DEBUG_MODE      Select debug app build (1 = debug, otherwise production; default: $(DEBUG_MODE))"
	@echo "  ROOT_DIR        Current root directory of the project"
	@echo "  ARCH            Target architecture, forwarded to the kernel build"
	@echo "  CROSS_COMPILE   Cross-compiler prefix, forwarded to both builds"
	@echo "  KDIR            Kernel build tree used by the driver"
