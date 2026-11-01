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
    _print_g "Usage: ${SCRIPT_NAME} [--verbose] [--bold] [--color] [options]"
    _print_g "       ${SCRIPT_NAME} --copy --files <files...> --directory <dst>"
    _print_g "       ${SCRIPT_NAME} --create --directory <dst>"
    _print_g "       ${SCRIPT_NAME} --print <green|blue|red|yellow> <string>"
    _print_g "       ${SCRIPT_NAME} --print-err <string>"
    _print_y "Options:"
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

_opt_error()
{
    _load_ansi_color
    print_err "$1"
}

_parser()
{
    _REST_ARGS=""
    _DO_FILES=""
    _DO_HELP=0

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
        --copy)
            _DO_COPY=1
            shift
            ;;
        --print)
            if [ "$#" -lt 3 ] || [ -z "${2}" ] || [ -z "${3}" ]; then
                _opt_error "Option '--print' requires a color and a string"
                printf 'Usage: %s --print <green|blue|red|yellow> <string>\n' "${SCRIPT_NAME}" >&2
                return 2
            fi
            _DO_PRINT_COLOR="${2}"
            _DO_PRINT_MSG="${3}"
            shift 3
            ;;
        --print-err)
            if [ "$#" -lt 2 ] || [ -z "${2}" ]; then
                _opt_error "Option '${1}' requires a string"
                return 2
            fi
            _DO_PRINT_ERR="${2}"
            shift 2
            ;;
        --files)
            if [ "$#" -lt 2 ] || [ -z "${2}" ] || [ "${2#-}" != "${2}" ]; then
                _opt_error "Option '--files' requires at least one file name"
                return 2
            fi
            shift
            while [ "$#" -gt 0 ] && [ "${1#-}" = "${1}" ]; do
                _DO_FILES="${_DO_FILES} ${1}"
                shift
            done
            ;;
        --directory=*)
            _DO_DIR="${1#--directory=}"
            if [ -z "${_DO_DIR}" ]; then
                _opt_error "Option '--directory' requires a directory path"
                return 2
            fi
            shift
            ;;
        --directory)
            if [ "$#" -lt 2 ] || [ -z "${2}" ]; then
                _opt_error "Option '--directory' requires a directory path"
                return 2
            fi
            _DO_DIR="${2}"
            shift 2
            ;;
        --help)
            _DO_HELP=1
            shift
            ;;
        --)
            shift
            _REST_ARGS="$*"
            return 0
            ;;
        *)
            _opt_error "Unknown option '$1'"
            usage
            return 2
            ;;
        esac
    done

    [ "${_DO_HELP:-0}" -eq 1 ] && return 3
    return 0
}

_main()
{
    if [ -n "${_DO_PRINT_COLOR}" ]; then
        _do_print "${_DO_PRINT_COLOR}" "${_DO_PRINT_MSG}" || return 1
    fi

    if [ -n "${_DO_PRINT_ERR}" ]; then
        print_err "${_DO_PRINT_ERR}"
    fi

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
    _parser "$@"
    rc=$?
    _load_ansi_color
    case "${rc}" in
    0) ;;
    3)
        usage
        return 0
        ;;
    *) return "${rc}" ;;
    esac

    _main
}

main "$@"
