#!/bin/bash

#===============================================================================
# Parsing command-line arguments with the getopts shell builtin
#===============================================================================
while getopts :f:e:v: option
do
	case $option in
		f) ARGUMENT_FILES="$OPTARG" ;;
		e) ARGUMENT_MAKESELF="$OPTARG" ;;
		v) ARGUMENT_VERSION="$OPTARG" ;;
	esac
done

#===============================================================================
# Checking if all required command-line arguments are provided
#===============================================================================
[ -z "${ARGUMENT_FILES}" ] && echo "$0: Missing argument: [-f files]" >&2
[ ! -e "${ARGUMENT_MAKESELF}" ] && echo "$0: Missing argument: [-e Makeself dir]" >&2

#===============================================================================
# Abort execution if required command-line argument is missing
#===============================================================================
[ -z "${ARGUMENT_FILES}" ] || [ ! -e "${ARGUMENT_MAKESELF}" ] && exit 1

#===============================================================================
# Checking if files or target path does not exists and abort
#===============================================================================
[ ! -d "${ARGUMENT_FILES}" ] && echo "$0: Files path does not exists." >&2 && exit 1

"${ARGUMENT_MAKESELF}" --notemp --sha256 "${ARGUMENT_FILES}" "${ARGUMENT_NAME}installer_${ARGUMENT_VERSION}.run" "${ARGUMENT_NAME} ${ARGUMENT_VERSION}"
