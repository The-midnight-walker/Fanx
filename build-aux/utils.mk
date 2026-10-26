# SPDX-License-Identifier: GPL-2.0
# vim: set ts=8 sw=8 noet tw=80 cc=80 fo+=t :

# =======================================
# OUTPUT STYLE WITH ANSI COLOR CHARACTERS
# =======================================
COLOR ?=
BOLD ?=

ifeq ($(BOLD),1)
    STYLE_PREFIX := \033[1;
else
    STYLE_PREFIX := \033[0;
endif

ifeq ($(COLOR),1)
    RED    := $(STYLE_PREFIX)31m
    GREEN  := $(STYLE_PREFIX)32m
    YELLOW := $(STYLE_PREFIX)33m
    BLUE   := $(STYLE_PREFIX)34m
    RESET  := \033[0m
else
    RED    :=
    GREEN  :=
    YELLOW :=
    BLUE   :=
    RESET  :=
endif

define print_err
    @printf '$(RED)%s$(RESET)\n' '$(1)' >&2
    exit 1
endef

define print_info
    @printf '$(GREEN)%s$(RESET)\n' '$(1)'
endef

define print_step
    @printf '==> $(BLUE)%s$(RESET)\n' '$(1)'
endef

define print_y
    @printf '$(YELLOW)%s$(RESET)\n' '$(1)'
endef

define print_g
    @printf '$(GREEN)%s$(RESET)\n' '$(1)'
endef

define print_b
    @printf '$(BLUE)%s$(RESET)\n' '$(1)'
endef