#!/bin/bash
set -e

#===============================================================================
# Parsing command-line arguments with the getopts shell builtin
#===============================================================================
while getopts :d:s:f: option
do
    case $option in
        s) ARGUMENT_DUMP_SYMS="$OPTARG" ;;
        f) ARGUMENT_FILES=("$OPTARG")
            until [[ $(eval "echo \${$OPTIND}") =~ ^-.* ]] || [ -z $(eval "echo \${$OPTIND}") ]; do
                ARGUMENT_FILES+=($(eval "echo \${$OPTIND}"))
                OPTIND=$((OPTIND + 1))
            done
            ;;
    esac
done

#===============================================================================
# Checking if all required command-line arguments are provided
#===============================================================================
[ -z "${ARGUMENT_DUMP_SYMS}" ] && echo "$0: Missing argument: [-s dump_syms]" >&2
[ -z "${ARGUMENT_FILES}" ] && echo "$0: Missing argument: [-f files]" >&2

[ -z "${ARGUMENT_DUMP_SYMS}" ] && exit 1

for val in "${ARGUMENT_FILES[@]}"; do
    [ ! -d "${val}" ] && echo "$0: File ${val} does not exists." >&2 && exit 1
done

#===============================================================================
# Prepare
#===============================================================================

# Dump symbols to file and strip
for val in "${ARGUMENT_FILES[@]}"; do
    "${ARGUMENT_DUMP_SYMS}" "${val}" > "${val}.sym"
    strip "${val}"
done
