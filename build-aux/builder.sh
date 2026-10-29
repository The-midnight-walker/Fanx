#!/bin/sh
# SPDX-License-Identifier: GPL-2.0
#
# vim: set ts=8 sw=8 noet tw=80 cc=80 fo+=t :
#
# DESCRIPTION
#       Automates build tasks and formatted printing
#
# EXIT STATUS
#   0    Success
#   1    General error
#   2    Invalid argument / usage error
#
# WARNINGS
#    - Some actions may require root privileges (sudo).
#    - May alter system package state via the package manager.
#
# AUTHORS
#    Midnight Walker <https://github.com/The-midnight-walker>

SCRIPT_NAME=$(basename -- "$0")
# Locate scripts directory and project root
ROOT_DIR=$(CDPATH='' cd -- "$(dirname -- "$0")/.." && pwd) || exit 1
SCRIPT_DIR="${ROOT_DIR}/scripts"

UTILS_SH="${SCRIPT_DIR}/utils.sh"

if [ ! -r "${UTILS_SH}" ]; then
    printf '%s: cannot read %s\n' "${SCRIPT_NAME}" "${UTILS_SH}" >&2
    exit 1
fi

# shellcheck source=../scripts/utils.sh
. "${UTILS_SH}"

usage()
{
    printf 'Usage: %s [--verbose] [--bold] [--color] [options]\n' "${SCRIPT_NAME}"
    printf '       %s --copy --files <files...> --directory <dst>\n' "${SCRIPT_NAME}"
    printf '       %s --create --directory <dst>\n' "${SCRIPT_NAME}"
    printf '       %s --print <green|blue|red|yellow> <string>\n' "${SCRIPT_NAME}"
    printf '       %s --print-err <string>\n\n' "${SCRIPT_NAME}"
    echo "Options:"
    echo "  --verbose                  Enable verbose informational messages"
    echo "  --bold                     Enable bold text styling in colors"
    echo "  --color                    Enable ANSI colored output"
    echo "  --print <color> <string>   Print a message in color (green, blue, red, yellow)"
    echo "  --print-err <string>       Print an error message in red to stderr"
    echo "  --copy                     Copy files (cp -p) to destination directory"
    echo "  --create                   Create a directory path (mkdir -p, mode 0700)"
    echo "  --directory <path>         Target directory path for --create or --copy"
    echo "  --files <file...>          List of source files to copy"
    echo "  --help                     Display this help message"
}

_do_print()
{
    _color="$1"
    _msg="$2"
    case "${_color}" in
    green) _print_g "${_msg}" ;;
    blue) _print_b "${_msg}" ;;
    red) _print_r "${_msg}" ;;
    yellow) _print_y "${_msg}" ;;
    *)
        print_err "Unknown color '${_color}': available colors are green, blue, red, yellow"
        return 1
        ;;
    esac
}

_parser()
{
    if [ "$#" -eq 0 ]; then
        usage
        return 2
    fi

    while [ "$#" -gt 0 ]; do
        case "$1" in
        # Flags
        --verbose)
            _DO_VERBOSE=1
            shift
            ;;
        --color)
            _DO_COLOR=1
            shift
            ;;
        --create)
            _DO_CRTDIR=1
            shift
            ;;
        --bold)
            _DO_BOLD=1
            shift
            ;;
        --print)
            if [ -z "${2}" ] || [ -z "${3}" ]; then
                _load_ansi_color
                print_err "Option '--print' requires a color and a string"
                printf 'Usage: %s --print <green|blue|red|yellow> <string>\n' "${SCRIPT_NAME}" >&2
                return 1
            fi
            _load_ansi_color
            _do_print "${2}" "${3}" || return 1
            shift 3
            ;;
        --print-err)
            if [ -z "${2}" ] || [ "${2#-}" != "${2}" ]; then
                _load_ansi_color
                print_err "Option '${1}' requires a string"
                return 1
            fi
            _load_ansi_color
            print_err "${2}"
            shift 2
            ;;
        --copy)
            _DO_COPY=1
            shift
            ;;
        --files)
            if [ -z "${2}" ] || [ "${2#-}" != "${2}" ]; then
                _load_ansi_color
                print_err "Option '--files' requires at least one file name"
                return 1
            fi
            shift
            while [ "${#}" -gt 0 ] && [ "${1#-}" = "${1}" ]; do
                _DO_FILES="${_DO_FILES} ${1}"
                shift
            done
            ;;
        --directory)
            if [ -z "${2}" ]; then
                _load_ansi_color
                print_err "Option '--directory' requires a directory path"
                return 1
            fi
            _DO_DIR="${2}"
            shift 2
            ;;
        --help)
            usage
            return 0
            ;;
        --)
            shift
            break
            ;;
        *)
            _load_ansi_color
            print_err "Unknown option '$1'"
            usage
            return 2
            ;;
        esac
    done

    return 0
}

_main()
{
    if [ -n "${_DO_CRTDIR}" ]; then
        if [ -z "${_DO_DIR}" ]; then
            print_err "--create requires --directory <dst>"
            return 1
        fi
        _create_dir "${_DO_DIR}" || return "$?"
    fi

    if [ -n "${_DO_COPY}" ]; then
        if [ -z "${_DO_FILES}" ] || [ -z "${_DO_DIR}" ]; then
            print_err "--copy requires --files and --directory"
            return 1
        fi
        for _file in ${_DO_FILES}; do
            _cp_file "${_file}" "${_DO_DIR}" || return "$?"
        done
    fi

    return 0
}

main()
{
    _parser "$@" || return "$?"
    _load_ansi_color
    _main
}

main "$@"
