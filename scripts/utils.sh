#!/bin/sh
# SPDX-License-Identifier: GPL-2.0
# vim: set ts=8 sw=8 noet tw=80 cc=80 fo+=t :

# ============
# OUTPUT STYLE
# ============
# ANSI color definitions (POSIX printf compatible)
_load_ansi_color()
{
    if [ "${_DO_COLOR:-0}" -eq 1 ]; then
        RED='\033[0;31m'
        GREEN='\033[0;32m'
        YELLOW='\033[0;33m'
        NC='\033[0m'
    else
        RED=''
        GREEN=''
        YELLOW=''
        NC=''
    fi
    OK="${GREEN}[ ok ]${NC}"
    NOT="${RED}[ X  ]${NC}"
}

_print_y()
{
    if [ "${_DO_COLOR:-0}" -eq 1 ]; then
        printf "%b%s%b\n" "${YELLOW}" "$1" "${NC}"
    else
        printf "%s\n" "$1"
    fi
}

_print_g()
{
    if [ "${_DO_COLOR:-0}" -eq 1 ]; then
        printf "%b%s%b\n" "${GREEN}" "$1" "${NC}"
    else
        printf "%s\n" "$1"
    fi
}

_print_r()
{
    if [ "${_DO_COLOR:-0}" -eq 1 ]; then
        printf "%b%s%b\n" "${RED}" "$1" "${NC}"
    else
        printf "%s\n" "$1"
    fi
}

print_err()
{
    # Print error message in red
    _print_r "Error: $1" >&2
}

print_success()
{
    # Print success message in green
    printf "%s" "---| "
    _print_g "$1"
}

print_info()
{
    # Print informational message in yellow only if verbose mode is enabled
    if [ "${_DO_VERBOSE:-0}" -eq 1 ]; then
        printf "%s" "---| "
        _print_y "$1"
    fi
}

# =============================
# RIGHTS, FILES AND PRIVILIEGES
# =============================

_is_login_root()
{
    # Verify that this script is running with root privileges
    # NOTE: `return 1` instead of `exit 1`.
    if [ "$(id -u)" -ne 0 ]; then
        print_err 'Not logged in as root to carry out this operation: Permission denied'
        return 1
    fi
    return 0
}

_add_right_x()
{
    # Add execution permission (+x) to a file or directory
    obj="$1"
    if [ ! -e "${obj}" ]; then
        print_err "file/directory '${obj}' does not exist"
        return 1
    fi
    if [ ! -x "${obj}" ]; then
        print_info "*** chmod +x '${obj}' ***"
        chmod +x "${obj}" 2>/dev/null || {
            print_err "Failed to chmod +x '${obj}'"
            return 1
        }
    fi
    return 0
}

_add_right_w()
{
    # Add write permission (+w) to a file or directory
    obj="$1"

    if [ ! -e "${obj}" ]; then
        print_err "file/directory '${obj}' does not exist"
        return 1
    fi

    if [ ! -w "${obj}" ]; then
        print_info "*** chmod +w '${obj}' ***"
        chmod +w "${obj}" 2>/dev/null || {
            print_err "Failed to chmod +w '${obj}'"
            return 1
        }
    fi

    return 0
}

_add_right_r()
{
    # Add read permission (+r) to a file or directory
    obj="$1"

    if [ ! -e "${obj}" ]; then
        print_err "file/directory '${obj}' does not exist"
        return 1
    fi

    if [ ! -r "${obj}" ]; then
        print_info "*** chmod +r '${obj}' ***"
        chmod +r "${obj}" 2>/dev/null || {
            print_err "Failed to chmod +r '${obj}'"
            return 1
        }
    fi

    return 0
}

_create_dir()
{
    # Create a directory path with private user-only
    # permissions (0700) on all created levels
    dir="$1"

    if [ ! -d "${dir}" ]; then
        print_info "*** mkdir -m 0700 -p '${dir}' ***"
        # Temporarily restrict umask so intermediate parent
        # directories are also private
        if ! (umask 0077 && mkdir -p "${dir}") 2>/dev/null; then
            print_err "Failed to create directory '${dir}'"
            return 1
        fi
    fi

    return 0
}

_cp_file()
{
    _cpf_src="$1"
    _cpf_dest="$2"

    if [ ! -f "${_cpf_src}" ]; then
        print_err "Error: Source file '${_cpf_src}' does not exist."
        return 1
    fi

    if ! cp -p "${_cpf_src}" "${_cpf_dest}"; then
        print_err "Failed to copy '${_cpf_src}' to '${_cpf_dest}'." >&2
        return 1
    fi

    return 0
}
