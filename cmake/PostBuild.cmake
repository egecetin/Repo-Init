# Update dist folder and prepare makeself archive
include(Colorize)
include(CMakeCacheForScript)

if (${CMAKE_BUILD_TYPE} STREQUAL "Release")

	message(STATUS "${BoldBlue}Updating dist folder and preparing packages${ColourReset}")
	# Clear old files
	execute_process(COMMAND "find" "${PROJECT_SOURCE_DIR}/dist/temp/" "-maxdepth" "1" "-type" "f" "-delete")

	# Copy shared libraries
	execute_process(COMMAND "sh"
							"${PROJECT_SOURCE_DIR}/scripts/ldd-copy-dependencies.sh"
							"-b"
							"${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}/${PROJECT_NAME}"
							"-t"
							"${PROJECT_SOURCE_DIR}/dist/temp"
							)

	# Create makeself package
	execute_process(COMMAND "sh"
							"${PROJECT_SOURCE_DIR}/scripts/makeself/makeself.sh"
							"--notemp"
							"--sha256"
							"${PROJECT_SOURCE_DIR}/dist/temp"
							"${PROJECT_NAME}.${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.run"
							"${PROJECT_NAME}"
							)

	# Move to another directory
	execute_process(COMMAND "mv"
							"-f"
							"${PROJECT_NAME}.${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.run"
							"${PROJECT_SOURCE_DIR}/dist/makeself/"
							)

	# Calculate MD5 checksums
	message(STATUS "${BoldBlue}Calculating MD5 checksums for binaries${ColourReset}")
	execute_process(COMMAND "find" "-type" "f" "-exec" "md5sum" "{}" "+"
					WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/dist/temp"
					OUTPUT_VARIABLE md5Str)

	# Calculate SHA1 checksums
	message(STATUS "${BoldBlue}Calculating SHA1 checksums for binaries${ColourReset}")
	execute_process(COMMAND "find" "-type" "f" "-exec" "sha1sum" "{}" "+"
					WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/dist/temp"
					OUTPUT_VARIABLE sha1Str)

	# Calculate SHA256 checksums
	message(STATUS "${BoldBlue}Calculating SHA256 checksums for binaries${ColourReset}")
	execute_process(COMMAND "find" "-type" "f" "-exec" "sha256sum" "{}" "+"
					WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/dist/temp"
					OUTPUT_VARIABLE sha256Str)

	# Write to file
	file(WRITE "${PROJECT_SOURCE_DIR}/dist/temp/checksum.md5" ${md5Str})
	file(WRITE "${PROJECT_SOURCE_DIR}/dist/temp/checksum.sha1" ${sha1Str})
	file(WRITE "${PROJECT_SOURCE_DIR}/dist/temp/checksum.sha256" ${sha256Str})

	message(STATUS "${BoldBlue}${PROJECT_NAME}.${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.run generated${ColourReset}")
endif()
