#!/bin/sh
# SPDX-License-Identifier: GPL-2.0
# vim: set ts=8 sw=8 noet tw=80 cc=80 fo+=t :
#
# DESCRIPTION
# *************************************************************
# Utility wrapper script for building, loading, unloading, and
# cleaning a Linux Kernel Module (LKM).
#
# Adapted and inspired by the LKM template utilities by Kaiwan N. Billimoria:
#   https://github.com/PacktPublishing/Linux-Kernel-Programming/blob/master/lkm
#
# Independent rewrite aligned with POSIX shell syntax standards.
# Licensed under GPLv2.
# *************************************************************
#
# EXIT STATUS
#   0    Success
#   1    General error
#   2    Invalid argument / usage error
#
# WARNINGS
#    - Actions here require root privileges.
#    - Clear the kernel buffer cache (comment out 'dmesg -C' in run_kmod
#           if you wish to keep previous logs)
# AUTHORS
#    Midnight Walker <https://github.com/The-midnight-walker>

SCRIPT_NAME=$(basename -- "$0")
# Locate scripts directory and project root
ROOT_DIR=$(CDPATH='' cd -- "$(dirname -- "$0")/.." && pwd) || exit 1
SCRIPT_DIR="${ROOT_DIR}/scripts"
#KERNEL_BUILD_DIR="${ROOT_DIR}/build/kernel"

_DO_RUN=""

UTILS_SH="${SCRIPT_DIR}/utils.sh"
if [ ! -r "${UTILS_SH}" ]; then
    printf '%s: cannot read %s\n' "${SCRIPT_NAME}" "${UTILS_SH}" >&2
    exit 1
fi

# shellcheck source=../scripts/utils.sh
. "${UTILS_SH}"

usage()
{
    _print_g "Usage: ${SCRIPT_NAME} [options] <kernel_module_name>"
    _print_y "Options:"
    echo "  -v, --verbose              Enable verbose informational messages"
    echo "  -c, --color                Enable ANSI colored output"
    echo "  -b, --bold                 Enable bold text styling in colors"
    echo "  -r, --run <modname>        Build and load the specified kernel \
module (WITHOUT extension)"
    echo "  -h, --help                 Display this help message"
}

print_banner()
{
    _print_b "      +---------------------------+"
    _print_b "      + LKM (Linux Kernel Module) +"
    _print_b "      +---------------------------+"
}

_make_build()
{
    [ -f "${_kmod_name}.ko" ] && return 0

    # TODO: relace 'compile-bear' with 'build'
    make C="${_DO_COLOR}" V="${_DO_VERBOSE}" B="${_DO_BOLD}" \
        MOD_NAME_C="${_kmod_name}" compile-bear

    [ -f "${_kmod_name}.ko" ] || {
        print_err "${SCRIPT_NAME}: ${_kmod_name}.ko was not built, cannot \
be or failed to be, aborting..."
        return 1
    }
}

_show_dmesg()
{
    if [ "${_DO_COLOR:-0}" -eq 1 ]; then
        dmesg --color=always
    else
        dmesg --color=never
    fi
}

_run_kmod()
{
    if [ "${_DO_VERBOSE}" -eq 1 ]; then
        if [ -r /etc/os-release ]; then
            printf "Distro: "
            _print_b "$(. /etc/os-release && echo "${PRETTY_NAME}")"
        fi
        printf "Kernel: "
        _print_b "$(uname -r)"
    fi

    _make_build || return 1

    if lsmod | grep -q "^${_kmod_name} "; then
        rmmod "${_kmod_name}" || return 1
    fi

    # Comment this
    dmesg -C

    insmod "./${_kmod_name}.ko" || {
        print_err "[!] ${SCRIPT_NAME}: insmod failed:"
        _show_dmesg >&2
    }

    if [ "${_DO_VERBOSE}" -eq 1 ]; then
        printf "module: "
        _loaded="$(lsmod | grep "^${_kmod_name} " | xargs)"
        [ -n "${_loaded}" ] && _print_y "${_loaded}"
    fi
    echo "dmesg: "

    _show_dmesg
}

_set_kmod()
{
    case "$1" in
    '' | *.* | */*)
        _opt_error "Specify the kernel module name ONLY \
(no path, no extension)."
        return 2
        ;;
    esac
    _kmod_name="$1"
    _DO_RUN=1
    return 0
}

_opt_error()
{
    _load_ansi_color
    print_err "$1"
    usage
}

_parser()
{
    _REST_ARGS=""
    _DO_HELP=0

    if [ "$#" -eq 0 ]; then
        usage
        return 2
    fi

    while [ "$#" -gt 0 ]; do
        case "$1" in
        --)
            shift
            _REST_ARGS="$*"
            return 0
            ;;
        --verbose)
            _DO_VERBOSE=1
            shift
            ;;
        --color)
            _DO_COLOR=1
            shift
            ;;
        --bold)
            _DO_BOLD=1
            shift
            ;;
        --help) _DO_HELP=1 ;;
        --run=*)
            _set_kmod "${1#--run=}" || return $?
            shift
            ;;
        --run)
            if [ "$#" -lt 2 ]; then
                _opt_error "Option --run requires a module name argument."
                return 2
            fi
            _set_kmod "$2" || return $?
            shift 2
            ;;
        --*)
            _opt_error "Unknown option '$1'"
            return 2
            ;;
        -?*)
            _opts="${1#-}"
            shift
            while [ -n "${_opts}" ]; do
                _char="${_opts%"${_opts#?}"}"
                _opts="${_opts#?}"
                case "${_char}" in
                v) _DO_VERBOSE=1 ;;
                c) _DO_COLOR=1 ;;
                b) _DO_BOLD=1 ;;
                h) _DO_HELP=1 ;;
                r)
                    if [ -z "${_opts}" ]; then
                        if [ "$#" -eq 0 ]; then
                            _opt_error "Option -r requires a module name \
argument."
                            return 2
                        fi
                        _opts="$1"
                        shift
                    fi
                    _set_kmod "${_opts}" || return $?
                    _opts=""
                    ;;
                *)
                    _opt_error "Unknown option '-${_char}'"
                    return 2
                    ;;
                esac
            done
            ;;
        *)
            _opt_error "Unknown option '$1'"
            return 2
            ;;
        esac
    done

    [ "${_DO_HELP:-0}" -eq 1 ] && return 3
    return 0
}

_main()
{
    print_banner

    if [ -n "${_DO_RUN}" ]; then
        _is_login_root || return "$?"
        _run_kmod || return "$?"
    fi
    return 0
}

main()
{
    _parser "$@"
    rc=$?
    _load_ansi_color
    case "${rc}" in
    0) ;;
    3)
        print_banner
        usage
        return 0
        ;;
    *) return "${rc}" ;;
    esac

    _main
}

main "$@"
